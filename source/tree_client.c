/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "client_stub-private.h"
#include "client_stub.h"
#include "message-private.h"
#include "op_status-private.h"
#include "tree_client-private.h"
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h>

#define PATH_BUF_LEN 32
#define ZOO_DATA_LEN 32
#define OP_MAX_ATTEMPTS 3

typedef struct String_vector zoo_string;

static const int TIMEOUT = 3000; // in ms
static const char root_path[] = "/chain";
static char w_context[] = "Head/Tail Server Watcher";
static zhandle_t *zh;

static struct rtree_t *head;
static struct rtree_t *tail;

static char *line;

int compare_fn(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

int tail_verify(int op_n)
{

    for (int i = 1; i < 4; i++)
    {
        if (rtree_verify(tail, op_n) == OP_SUCCESSFUL)
        {
            return OP_SUCCESSFUL;
        }

        usleep(i * 250000); // espera i * 250ms
    }
    return -1;
}

void update_head_tail(zoo_string *children_list)
{
    if (children_list->count == 0)
    {
        return; // ! Sem servidores mas "/chain" existe
    }
    int buf_len = ZOO_DATA_LEN;
    char address_port[PATH_BUF_LEN];
    char head_path[PATH_BUF_LEN];
    char head_host[ZOO_DATA_LEN];
    char tail_path[PATH_BUF_LEN];
    char tail_host[ZOO_DATA_LEN];

    qsort(children_list->data, children_list->count, sizeof(char *), compare_fn);

    if (head != NULL)
    {
        sprintf(head_host, "%s:%d", head->address, head->port);
    }

    if (tail != NULL)
    {
        sprintf(tail_host, "%s:%d", tail->address, tail->port);
    }

    sprintf(head_path, "%s/%s", root_path, children_list->data[0]);
    sprintf(tail_path, "%s/%s", root_path, children_list->data[children_list->count - 1]);

    if (ZOK != zoo_get(zh, head_path, 0, address_port, &buf_len, NULL))
    {
        fprintf(stderr, "update_head_tail: Error getting data at '%s'.\n", head_path);
        return;
    }
    if (address_port != NULL)
    {
        if (head != NULL && strcmp(head->znode_id, head_path) != 0)
        {
            rtree_disconnect(head); // ! verificar erros
            head = NULL;
        }
        if (head == NULL)
        {
            head = rtree_connect(address_port);
            head->znode_id = strdup(head_path);
        }
    }

    memset(address_port, 0, buf_len);
    buf_len = ZOO_DATA_LEN;

    if (ZOK != zoo_get(zh, tail_path, 0, address_port, &buf_len, NULL))
    {
        fprintf(stderr, "update_head_tail: Error getting data at '%s'.\n", tail_path);
        return;
    }
    if (address_port != NULL)
    {
        if (tail != NULL && strcmp(tail->znode_id, tail_path) != 0)
        {
            rtree_disconnect(tail); // ! verificar erros
            tail = NULL;
        }
        if (tail == NULL)
        {
            tail = rtree_connect(address_port);
            tail->znode_id = strdup(tail_path);
        }
    }
}

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
                update_head_tail(children_list);
            }
        }
    }
    for (int i = 0; i < children_list->count; i++)
    {
        free(children_list->data[i]);
    }
    free(children_list);
}

int main(int argc, char const *argv[])
{
    /* Testar os argumentos de entrada */
    if (argc != 2)
    {
        printf("Uso: ./tree-client <zk_server>:<zk_port>\n");
        printf("Exemplo de uso: ./tree-client 127.0.0.1:2181\n");
        exit(EXIT_FAILURE);
    }
    size_t line_size = 20; // valor inicial, cresce consoante o necessário
    line = malloc(line_size);
    if (line == NULL)
    {
        exit(EXIT_FAILURE);
    }

    zh = zookeeper_init(argv[1], NULL, TIMEOUT, 0, NULL, 0);
    if (zh == NULL)
    {
        perror("main");
        free(line);
        exit(EXIT_FAILURE);
    }

    printf("Ligação estabelecida com o servidor ZooKeeper@%s\n", argv[1]);

    child_watcher(zh, ZOO_CHILD_EVENT, ZOO_CONNECTED_STATE, root_path, w_context);

    signal_sigint(tree_client_exit);

    struct pollfd desc_set[3];
    struct pollfd *stdin_desc = &desc_set[0];
    struct pollfd *head_desc = &desc_set[1];
    struct pollfd *tail_desc = &desc_set[2];
    stdin_desc->fd = fileno(stdin); // stdin (cliente)
    stdin_desc->events = POLLIN;
    head_desc->fd = head->sockfd; // ligação com o servidor HEAD
    head_desc->events = POLLIN;
    tail_desc->fd = tail->sockfd; // ligação com o servidor TAIL
    tail_desc->events = POLLIN;

    // esperamos por input do cliente, ou fecho do servidor
    while (poll(desc_set, sizeof(desc_set) / sizeof(struct pollfd), -1) > 0)
    {
        if (tail_desc->revents & POLLHUP)
        {
            printf("O servidor terminou a ligação.\n");
            break;
        }
        if (tail_desc->revents & POLLIN) // recebemos mensagem do servidor
        {
            if (available_read_bytes(tail_desc->fd) == 0) // 0 bytes para ler
            {
                printf("O servidor terminou a ligação.\n");
                break;
            }
        }

        if ((stdin_desc->revents & POLLIN) == 0)
        {
            continue;
        }
        if ((line_size = getline(&line, &line_size, stdin)) == -1)
        {
            perror("main");
            tree_client_exit();
        }
        line[line_size - 1] = '\0';
        char *token = strtok(line, " ");
        if (token == NULL)
        {
            printf("Comando não reconhecido. Tente novamente.\n");
            continue;
        }

        if (strcmp(token, "put") == 0)
        {
            char *key, *value;

            token = strtok(NULL, " ");
            if (token == NULL)
            {
                printf("Erro a ler argumentos.\n");
                continue;
            }
            key = strdup(token);

            token = strtok(NULL, "\0");
            if (token == NULL)
            {
                free(key);
                printf("Erro a ler argumentos.\n");
                continue;
            }
            value = strdup(token);

            struct data_t *data = data_create2(strlen(value) + 1, value);
            struct entry_t *entry = entry_create(key, data);

            int op_n = rtree_put(head, entry);
            if (op_n == -1)
            {
                printf("Erro no comando 'put'.\n");
            }
            else
            {
                printf("'%d': id da operacão 'put' para a entrada '%s'\n", op_n, entry->key);
                for (int i = 1; i <= OP_MAX_ATTEMPTS; i++)
                {
                    if (tail_verify(op_n) == OP_SUCCESSFUL)
                    {
                        printf("'%d': operação verificada na tail com sucesso\n", op_n);
                        break;
                    }
                    printf("'%d': operação não se verificou na tail (tentativa %d/%d)\n", op_n, i, OP_MAX_ATTEMPTS);
                    op_n = rtree_put(head, entry);
                }
            }
            entry_destroy(entry);
        }
        else if (strcmp(token, "get") == 0)
        {
            char *key = strtok(NULL, " ");
            if (key == NULL)
            {
                printf("Erro a ler argumentos.\n");
                continue;
            }

            struct data_t *data = rtree_get(tail, key);
            if (data == NULL)
            {
                printf("Erro no comando 'get'.\n");
            }
            else if (data->datasize == 0)
            {
                printf("Não existe entrada associada à chave '%s'.\n", key);
            }
            else
            {
                printf("'%s': ", key);
                print_value(data);
                printf("\n");
            }
            data_destroy(data);
        }
        else if (strcmp(token, "del") == 0)
        {
            char *key = strtok(NULL, " ");
            if (key == NULL)
            {
                printf("Erro a ler argumentos.\n");
                continue;
            }

            int op_n = rtree_del(head, key);
            if (op_n == -1)
            {
                printf("'%s': entrada inexistente\n", key);
                break;
            }
            else
            {
                printf("'%d': id da operacão 'del' para a entrada '%s'\n", op_n, key);
                for (int i = 1; i <= OP_MAX_ATTEMPTS; i++)
                {
                    if (tail_verify(op_n) == OP_SUCCESSFUL)
                    {
                        printf("'%d': operação verificada na tail com sucesso\n", op_n);
                        break;
                    }
                    printf("'%d': operação não se verificou na tail (tentativa %d/%d)\n", op_n, i, OP_MAX_ATTEMPTS);
                    op_n = rtree_del(head, key);
                }
            }
        }
        else if (strcmp(token, "verify") == 0)
        {
            char *key = strtok(NULL, " ");
            if (key == NULL)
            {
                printf("Erro a ler argumentos.\n");
                continue;
            }
            int n_op = atoi(key);
            if (n_op <= 0)
            {
                printf("'%s': identificador de operação inválido\n", key);
                continue;
            }

            int i = rtree_verify(tail, n_op);
            if (i == OP_UNAVAILABLE)
            {
                printf("'%d': número não associado a uma operação\n", n_op);
            }
            else if (i == OP_SUCCESSFUL)
            {
                printf("'%d': operação executada\n", n_op);
            }
            else if (i == OP_WAITING)
            {
                printf("'%d': operação ainda não foi executada\n", n_op);
            }
        }
        else if (strcmp(token, "size") == 0)
        {
            int size = rtree_size(tail);
            if (size == -1)
            {
                printf("Erro no comando 'size'.\n");
            }
            else
            {
                printf("Tamanho: %d\n", size);
            }
        }
        else if (strcmp(token, "height") == 0)
        {
            int height = rtree_height(tail);
            if (height == -1)
            {
                printf("Erro no comando 'height'.\n");
            }
            else
            {
                printf("Altura: %d\n", height);
            }
        }
        else if (strcmp(token, "getkeys") == 0)
        {
            char **keys = rtree_get_keys(tail);
            if (keys == NULL)
            {
                printf("Erro no comando 'getkeys'.\n");
            }
            else if (keys[0] != NULL)
            {
                printf("Chaves: [%s", keys[0]);
                for (int i = 1; keys[i] != NULL; i++)
                {
                    printf(", %s", keys[i]);
                }
                printf("]\n");
            }
            else
            {
                printf("Arvore vazia.\n");
            }
            rtree_free_keys(keys);
        }
        else if (strcmp(token, "getvalues") == 0)
        {
            struct data_t **values = (struct data_t **)rtree_get_values(tail);
            if (values == NULL)
            {
                printf("Erro no comando 'getvalues'.\n");
            }
            else if (values[0] != NULL)
            {
                printf("Valores: [");
                print_value(values[0]);
                for (int i = 1; values[i] != NULL; i++)
                {
                    printf(", ");
                    print_value(values[i]);
                }
                printf("]\n");
            }
            else
            {
                printf("Arvore vazia.\n");
            }
            rtree_free_values((void **)values);
        }
        else if (strcmp(token, "quit") == 0 || strcmp(token, "exit") == 0)
        {
            break;
        }
        else
        {
            printf("Comando nao reconhecido. Tente novamente.\n");
        }
    }

    tree_client_exit();
}

void print_value(struct data_t *data)
{
    char *str = (char *)data->data;
    if (str[data->datasize - 1] == '\0')
    {
        printf("'%s'", str); // imprime dados null-terminated
    }
    else
    {
        printf("desconhecido (%d bytes)", data->datasize);
    }
}

void tree_client_exit()
{
    zookeeper_close(zh);
    int status = 0;
    if (head == NULL)
    {
        free(line);
        exit(status);
    }
    if (head != tail)
    {
        status = rtree_disconnect(head);
    }
    status |= rtree_disconnect(tail);
    if (status != 0)
    {
        perror("tree_client");
    }
    free(line);
    exit(status);
}
