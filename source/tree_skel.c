/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "tree_skel.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "entry.h"
#include "op_status-private.h"
#include "string.h"
#include "tree.h"
#include "tree_skel-private.h"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h>

#define PATH_BUF_LEN 32
#define ZOO_DATA_LEN 32

static const int TIMEOUT = 3000; // in ms
static const char root_path[] = "/chain";
static const char znode_prefix[] = "/chain/node";
static char w_context[] = "Next Server Watcher";
static zhandle_t *zh = NULL;

static char znode_id[PATH_BUF_LEN];
static struct rtree_t *next_server;

uint8_t terminate = 0;

struct tree_t *tree;
request_t *queue_head;
int last_assigned = 1;
op_proc_t op_proc;

int num_threads;
pthread_t *thread;
int *thread_param;

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t op_proc_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx)
{
    zoo_string *children_list = malloc(sizeof(zoo_string));
    if (children_list == NULL)
    {
        perror("child_watcher");
        exit(EXIT_FAILURE);
    }
    if (state == ZOO_CONNECTED_STATE)
    {
        if (type == ZOO_CHILD_EVENT)
        {
            if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list))
            {
                fprintf(stderr, "child_watcher: Error getting data at '%s'.\n", root_path);
            }
            else
            {
                update_next_server(children_list);
            }
        }
    }
    for (int i = 0; i < children_list->count; i++)
    {
        free(children_list->data[i]);
    }
    free(children_list);
}

int tree_skel_init(int N)
{
    num_threads = N;
    tree = tree_create();
    if (tree == NULL)
    {
        return -1;
    }
    op_proc.max_proc = 0;                                   // default pois op_n >= 1
    op_proc.in_progress = calloc(sizeof(int), num_threads); // max N threads a concorrer
    if (op_proc.in_progress == NULL)
    {
        perror("tree_skel_init");
        return -1;
    }
    thread = malloc(sizeof(pthread_t) * num_threads);
    if (thread == NULL)
    {
        perror("tree_skel_init");
        return -1;
    }
    thread_param = malloc(sizeof(int) * num_threads);
    if (thread_param == NULL)
    {
        perror("tree_skel_init");
        return -1;
    }
    for (int i = 0; i < num_threads; i++)
    {
        thread_param[i] = i; // para aceder a op_proc.in_progress[i]
        if (pthread_create(&thread[i], NULL, &process_request, (void *)&thread_param[i]) != 0)
        {
            perror("tree_skel_init");
            return -1;
        }
    }
    return 0;
}

void tree_skel_destroy()
{
    terminate = 1;
    pthread_cond_broadcast(&queue_cond);
    zookeeper_close(zh);
    if (next_server != NULL)
    {
        rtree_disconnect(next_server);
    }

    int *r;
    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_join(thread[i], (void **)&r) != 0)
        {
            perror("tree_skel_destroy");
            exit(EXIT_FAILURE);
        }
    }
    free(thread_param);
    free(thread);
    free(op_proc.in_progress);
    request_t *temp;
    while (queue_head != NULL)
    {
        temp = queue_head->next;
        free(queue_head->key);
        data_destroy(queue_head->data);
        free(queue_head);
        queue_head = temp;
    }
    tree_destroy(tree);
}

int invoke(MessageT *msg)
{
    int status = 0;

    if (msg == NULL)
    {
        return -1;
    }

    switch (msg->opcode)
    {
    case MESSAGE_T__OPCODE__OP_SIZE: {
        pthread_mutex_lock(&tree_lock);
        int size = tree_size(tree);
        pthread_mutex_unlock(&tree_lock);
        msg->data.len = sizeof(int);
        msg->data.data = malloc(sizeof(int));
        *((int *)msg->data.data) = size;
        msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        break;
    }

    case MESSAGE_T__OPCODE__OP_HEIGHT: {
        pthread_mutex_lock(&tree_lock);
        int height = tree_height(tree);
        pthread_mutex_unlock(&tree_lock);
        msg->data.len = sizeof(int);
        msg->data.data = malloc(sizeof(int));
        *((int *)msg->data.data) = height;
        msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        break;
    }

    case MESSAGE_T__OPCODE__OP_DEL: {
        char *key = (char *)msg->data.data;
        request_t *request = create_request(last_assigned, OP_DEL, key, NULL);
        free(msg->data.data);
        if (request == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->data.len = 0;
            msg->data.data = NULL;
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->data.len = sizeof(int);
            msg->data.data = malloc(msg->data.len);
            if (msg->data.data == NULL)
            {
                perror("invoke");
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->data.len = 0;
            }
            else
            {
                *((int *)msg->data.data) = last_assigned;
                last_assigned++;
                queue_add_request(request);
            }
        }
        break;
    }

    case MESSAGE_T__OPCODE__OP_GET: {
        char *key = (char *)msg->data.data;
        pthread_mutex_lock(&tree_lock);
        struct data_t *value = tree_get(tree, key);
        pthread_mutex_unlock(&tree_lock);

        if (value == NULL)
        {
            printf("get: Chave '%s' não encontrada.\n", key);
            value = calloc(1, sizeof(struct data_t)); // size=0, data=NULL
        }

        free(msg->data.data);
        DataT data = DATA_T__INIT;
        data.data.len = value->datasize;
        data.data.data = value->data;
        msg->data.len = data_t__get_packed_size(&data);
        msg->data.data = malloc(msg->data.len);
        data_t__pack(&data, msg->data.data);
        data_destroy(value);
        msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        break;
    }

    case MESSAGE_T__OPCODE__OP_PUT: {
        EntryT *entry = entry_t__unpack(NULL, msg->data.len, msg->data.data);
        struct data_t *data = data_create2(entry->value.len, entry->value.data);
        request_t *request = create_request(last_assigned, OP_PUT, entry->key, data);
        free(data);
        free(msg->data.data);
        entry_t__free_unpacked(entry, NULL);
        if (request == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->data.len = 0;
            msg->data.data = NULL;
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->data.len = sizeof(int);
            msg->data.data = malloc(msg->data.len);
            if (msg->data.data == NULL)
            {
                perror("invoke");
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->data.len = 0;
            }
            else
            {
                *((int *)msg->data.data) = last_assigned;
                last_assigned++;
                queue_add_request(request);
            }
        }
        break;
    }

    case MESSAGE_T__OPCODE__OP_GETKEYS: {
        KeysT keys = KEYS_T__INIT;
        pthread_mutex_lock(&tree_lock);
        keys.keys = tree_get_keys(tree);

        if (keys.keys == NULL)
        {
            pthread_mutex_unlock(&tree_lock);
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->data.len = 0;
            msg->data.data = NULL;
            break;
        }
        keys.n_keys = tree_size(tree);
        pthread_mutex_unlock(&tree_lock);
        msg->data.len = keys_t__get_packed_size(&keys);
        msg->data.data = malloc(msg->data.len);
        keys_t__pack(&keys, msg->data.data);
        tree_free_keys(keys.keys);
        msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        break;
    }

    case MESSAGE_T__OPCODE__OP_GETVALUES: {
        pthread_mutex_lock(&tree_lock);
        struct data_t **data_arr = (struct data_t **)tree_get_values(tree);
        if (data_arr == NULL)
        {
            pthread_mutex_unlock(&tree_lock);
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->data.len = 0;
            msg->data.data = NULL;
            break;
        }
        ValuesT *values = malloc(sizeof(ValuesT)); // free_unpacked requer memória dinâmica
        values_t__init(values);
        values->n_values = tree_size(tree);
        pthread_mutex_unlock(&tree_lock);
        values->values = malloc(sizeof(ProtobufCBinaryData) * values->n_values);
        for (int i = 0; i < values->n_values; i++)
        {
            values->values[i].len = data_arr[i]->datasize;
            values->values[i].data = data_arr[i]->data;
            free(data_arr[i]);
        }
        free(data_arr);
        msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;

        msg->data.len = values_t__get_packed_size(values);
        msg->data.data = malloc(msg->data.len);
        values_t__pack(values, msg->data.data);
        values_t__free_unpacked(values, NULL);
        break;
    }

    case MESSAGE_T__OPCODE__OP_VERIFY: {
        int op_n = *(int *)msg->data.data;
        free(msg->data.data);
        int status = verify(op_n);
        msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->data.len = sizeof(int);
        msg->data.data = malloc(msg->data.len);
        *((int *)msg->data.data) = status;
        break;
    }

    default:
        msg->opcode = MESSAGE_T__OPCODE__OP_BAD;
        msg->c_type = MESSAGE_T__C_TYPE__CT_BAD;
        msg->data.len = 0;
        msg->data.data = NULL;
        break;
    }

    return status;
}

int verify(int op_n)
{
    int status = OP_SUCCESSFUL;
    pthread_mutex_lock(&op_proc_lock);
    if (op_n <= 0 || op_n > op_proc.max_proc)
    {
        status = OP_UNAVAILABLE;
    }
    else
    {
        for (int i = 0; i < num_threads; i++)
        {
            if (op_proc.in_progress[i] == op_n)
            {
                status = OP_WAITING;
                break;
            }
        }
    }
    pthread_mutex_unlock(&op_proc_lock);
    return status;
}

void *process_request(void *params)
{
    int tid = *(int *)params;
    printf("Thread #%d: pronta a receber pedidos.\n", tid);
    while (!terminate)
    {
        request_t *request = queue_get_request();
        if (request == NULL)
        {
            break;
        }
        pthread_mutex_lock(&op_proc_lock);
        op_proc.in_progress[tid] = request->op_n;
        pthread_mutex_unlock(&op_proc_lock);

        printf("Thread #%d: pedido %d recebido.\n", tid, request->op_n);
        pthread_mutex_lock(&tree_lock);
        if (request->op == OP_DEL)
        {
            if (tree_del(tree, request->key) == -1)
            {
                printf("del: Chave '%s' não encontrada.\n", request->key);
            }
            if (next_server != NULL && (rtree_del(next_server, request->key) == -1))
            {
                printf("del: Erro ao submeter operação '%d' no servidor seguinte (%s:%d).\n", request->op_n,
                       next_server->address, next_server->port);
            }
        }
        else if (request->op == OP_PUT)
        {
            if ((tree_put(tree, request->key, request->data) == -1))
            {
                printf("put: Erro ao inserir a entrada com a chave '%s'.\n", request->key);
            }
            struct entry_t *entry = entry_create(request->key, request->data);
            if (next_server != NULL && (rtree_put(next_server, entry) == -1))
            {
                printf("put: Erro ao submeter operação '%d' no servidor seguinte (%s:%d).\n", request->op_n,
                       next_server->address, next_server->port);
            }
            free(entry);
        }
        pthread_mutex_unlock(&tree_lock);

        pthread_mutex_lock(&op_proc_lock);
        if (request->op_n > op_proc.max_proc)
        {
            op_proc.max_proc = request->op_n;
        }
        op_proc.in_progress[tid] = 0;
        pthread_mutex_unlock(&op_proc_lock);

        free(request->key);
        data_destroy(request->data);
        free(request);
        printf("[INFO] Thread #%d: pedido processado!\n", tid);
    }
    return 0;
}

void queue_add_request(request_t *request)
{
    pthread_mutex_lock(&queue_lock);
    if (queue_head == NULL)
    {
        queue_head = request;
        request->next = NULL;
    }
    else
    {
        request_t *rptr = queue_head;
        while (rptr->next != NULL)
        {
            rptr = rptr->next;
        }
        rptr->next = request;
        request->next = NULL;
    }
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_lock);
}

request_t *queue_get_request()
{
    pthread_mutex_lock(&queue_lock);
    while (!terminate && queue_head == NULL)
    {
        pthread_cond_wait(&queue_cond, &queue_lock);
    }
    if (terminate)
    {
        pthread_cond_signal(&queue_cond);
        pthread_mutex_unlock(&queue_lock);
        return NULL;
    }
    request_t *request = queue_head;
    queue_head = request->next;
    pthread_mutex_unlock(&queue_lock);
    return request;
}

request_t *create_request(int op_n, int op, char *key, struct data_t *data)
{
    request_t *request = malloc(sizeof(request_t));
    if (request == NULL)
    {
        perror("create_request");
        return NULL;
    }
    if ((request->key = strdup(key)) == NULL)
    {
        perror("create_request");
        free(request);
        return NULL;
    }
    request->data = data_dup(data);
    request->op_n = op_n;
    request->op = op;
    return request;
}

char *get_if_addr(char **allowed_ifs, int n_ifs)
{
    struct ifaddrs *addrs;
    struct ifaddrs *temp;
    char *address;
    getifaddrs(&addrs);
    temp = addrs;

    while (temp != NULL)
    {
        if (temp->ifa_addr && temp->ifa_addr->sa_family == AF_INET)
        {
            for (int i = 0; i < n_ifs; i++)
            {
                if (strcmp(temp->ifa_name, allowed_ifs[i]) == 0)
                {
                    address = strdup(inet_ntoa(((struct sockaddr_in *)temp->ifa_addr)->sin_addr));
                    freeifaddrs(addrs);
                    return address;
                }
            }
        }

        temp = temp->ifa_next;
    }

    freeifaddrs(addrs);
    return NULL;
}

int tree_skel_zookeeper_init(const char *zk_address_port, short port)
{
    char *allowed_ifs[] = {"eth0", "enp0s3", "wlo1", "wlan0"};
    int n_ifs = sizeof(allowed_ifs) / sizeof(char *);
    char address_port[ZOO_DATA_LEN];
    char *address = get_if_addr(allowed_ifs, n_ifs);
    sprintf(address_port, "%s:%hu", address, port);
    free(address);

    // * Ligar ao zookeeper
    zh = zookeeper_init(zk_address_port, NULL, TIMEOUT, 0, NULL, 0);
    if (zh == NULL)
    {
        perror("tree_skel_zookeeper_init");
        return -1;
    }

    printf("Ligação estabelecida com o servidor ZooKeeper@%s\n", zk_address_port);

    // * CRIAR /CHAIN
    if (ZNONODE == zoo_exists(zh, root_path, 0, NULL))
    {
        if (ZOK != zoo_create(zh, root_path, NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0))
        {
            perror("tree_skel_zookeeper_init");
            return -1;
        }
    }

    char path_buffer[PATH_BUF_LEN];
    // * Criar znode
    if (ZOK != zoo_create(zh, znode_prefix, address_port, strlen(address_port) + 1, &ZOO_OPEN_ACL_UNSAFE,
                          ZOO_EPHEMERAL | ZOO_SEQUENCE, path_buffer, PATH_BUF_LEN))
    {
        perror("tree_skel_zookeeper_init");
        return -1;
    }
    strcpy(znode_id, path_buffer + sizeof(root_path)); // * Omite-se o prefixo /chain
    child_watcher(zh, ZOO_CHILD_EVENT, ZOO_CONNECTED_STATE, root_path, w_context);

    return 0;
}

int compare_fn(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void update_next_server(zoo_string *children_list)
{
    int buf_len = ZOO_DATA_LEN;
    char *candidate_id = NULL;
    char candidate_path[PATH_BUF_LEN];
    char address_port[ZOO_DATA_LEN];

    if (candidate_path == NULL)
    {
        perror("update_next_server");
        return;
    }
    qsort(children_list->data, children_list->count, sizeof(char *), compare_fn);

    for (int i = 0; i < children_list->count; i++)
    {
        if (strcmp(children_list->data[i], znode_id) > 0)
        {
            candidate_id = children_list->data[i];
            break;
        }
    }

    if (candidate_id != NULL)
    {
        if (next_server != NULL)
        {
            if (strcmp(candidate_id, next_server->znode_id) == 0) // next_server mantém-se
                return;
            else
                rtree_disconnect(next_server);
        }

        sprintf(candidate_path, "%s/%s", root_path, candidate_id);
        if (ZOK != zoo_get(zh, candidate_path, 0, address_port, &buf_len, NULL))
        {
            fprintf(stderr, "update_next_server: Error getting data at '%s'.\n", candidate_path);
        }
        else
        {
            usleep(500000);
            next_server = rtree_connect(address_port);
            next_server->znode_id = strdup(candidate_id);
        }
    }
    else if (next_server != NULL) // este server passa a ser TAIL
    {
        rtree_disconnect(next_server);
        next_server = NULL;
    }
}
