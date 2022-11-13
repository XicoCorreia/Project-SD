/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "tree_skel.h"
#include "entry.h"
#include "string.h"
#include "tree.h"
#include "tree_skel-private.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct tree_t *tree;
int num_threads;
request_t *queue_head;
int last_assigned = 1;
op_proc_t op_proc;

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t op_proc_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t op_proc_cond = PTHREAD_COND_INITIALIZER;

int tree_skel_init(int N)
{
    tree = tree_create();
    if (tree == NULL)
    {
        return -1;
    }
    op_proc.max_proc = 0; // default pois op_n >= 1
    num_threads = N;
    op_proc.in_progress = calloc(sizeof(int), N); // max N threads a concorrer
    if (op_proc.in_progress == NULL)
    {
        tree_skel_destroy(tree);
        return -1;
    }
    return 0;
}

void tree_skel_destroy()
{
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
        int size = tree_size(tree);
        msg->data.len = sizeof(int);
        msg->data.data = malloc(sizeof(int));
        *((int *)msg->data.data) = size;
        msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        break;
    }

    case MESSAGE_T__OPCODE__OP_HEIGHT: {
        int height = tree_height(tree);
        msg->data.len = sizeof(int);
        msg->data.data = malloc(sizeof(int));
        *((int *)msg->data.data) = height;
        msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        break;
    }

    case MESSAGE_T__OPCODE__OP_DEL: {
        // TODO
        char *key = (char *)msg->data.data;
        status = tree_del(tree, key);

        if (status < 0)
        {
            printf("del: Chave '%s' nao encontrada.\n", key);
            free(msg->data.data);
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->data.data = malloc(sizeof(int));
            if (msg->data.data == NULL)
            {
                msg->data.len = 0;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                break;
            }
            msg->data.len = sizeof(int);
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            *((int *)msg->data.data) = status;
        }
        else
        {
            free(msg->data.data);
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->data.len = 0;
            msg->data.data = NULL;
        }

        break;
    }

    case MESSAGE_T__OPCODE__OP_GET: {
        char *key = (char *)msg->data.data;
        struct data_t *value = tree_get(tree, key);

        if (value == NULL)
        {
            printf("get: Chave '%s' nao encontrada.\n", key);
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
        // TODO
        EntryT *entry = entry_t__unpack(NULL, msg->data.len, msg->data.data);
        struct data_t *value = data_create2(entry->value.len, entry->value.data);
        status = tree_put(tree, entry->key, value);
        free(msg->data.data);
        free(value);
        entry_t__free_unpacked(entry, NULL);

        msg->data.len = 0;
        msg->data.data = NULL;

        if (status < 0)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
        }

        msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        break;
    }

    case MESSAGE_T__OPCODE__OP_GETKEYS: {
        KeysT keys = KEYS_T__INIT;
        keys.keys = tree_get_keys(tree);

        if (keys.keys == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->data.len = 0;
            msg->data.data = NULL;
            break;
        }

        keys.n_keys = tree_size(tree);
        msg->data.len = keys_t__get_packed_size(&keys);
        msg->data.data = malloc(msg->data.len);
        keys_t__pack(&keys, msg->data.data);
        tree_free_keys(keys.keys);
        msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        break;
    }

    case MESSAGE_T__OPCODE__OP_GETVALUES: {
        struct data_t **data_arr = (struct data_t **)tree_get_values(tree);
        if (data_arr == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->data.len = 0;
            msg->data.data = NULL;
            break;
        }
        ValuesT *values = malloc(sizeof(ValuesT)); // free_unpacked requer memória dinâmica
        values_t__init(values);
        values->n_values = tree_size(tree);
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
        // TODO
        int op_n = *(int *)msg->data.data;
        int status = verify(op_n);
        msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->data.len = sizeof(int);
        msg->data.data = malloc(msg->data.len);
        *((int *)msg->data.data) = status;
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
    int status = 0;
    pthread_mutex_lock(&op_proc_lock);
    if (op_n > op_proc.max_proc)
    {
        status = -1;
    }
    else
    {
        for (int i = 0; i < num_threads; i++)
        {
            if (op_proc.in_progress[i] == op_n)
            {
                status = -1;
                break;
            }
        }
    }
    pthread_cond_signal(&op_proc_cond);
    pthread_mutex_unlock(&op_proc_lock);
    return status;
}

void *process_request(void *params)
{
    // TODO
    // ? considerar um thread_data_t com tipo (del vs. put) e num da thread
    request_t *request = queue_get_request();
    return request;
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
    while (queue_head == NULL)
    {
        pthread_cond_wait(&queue_cond, &queue_lock);
    }
    request_t *request = queue_head;
    queue_head = request->next;
    pthread_mutex_unlock(&queue_lock);
    return request;
}
