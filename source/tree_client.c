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

static const int TIMEOUT = 3000; // in ms
static const char *root_path = "/chain";
static char *w_context = "Head/Tail Server Watcher";
static char *line;
static struct rtree_t *head;
static struct rtree_t *tail;
static zhandle_t *zh;

typedef struct String_vector zoo_string;

void update_server_list(zoo_string *children_list)
{
    int len = 32;
    char address_port[len];
    char head_path[32];
    char head_host[32];
    char tail_path[32];
    char tail_host[32];

    sprintf(head_host, "%s:%d", head->address, head->port);
    sprintf(head_path, "%s/%s", root_path, children_list->data[0]);
    if (ZOK != zoo_get(zh, head_path, 0, address_port, &len, NULL))
    {
        fprintf(stderr, "update_server_list: Error getting data at '%s'.\n", head_path);
        return;
    }
    if (len > 0 && strcmp(head_host, address_port) != 0)
    {
        rtree_disconnect(head); // ! verificar erros
        head = rtree_connect(address_port);
    }

    memset(address_port, 0, len);
    len = 32;
    sprintf(tail_host, "%s:%d", tail->address, tail->port);
    sprintf(tail_path, "%s/%s", root_path, children_list->data[children_list->count - 1]);
    if (ZOK != zoo_get(zh, tail_path, 0, address_port, &len, NULL))
    {
        fprintf(stderr, "update_server_list: Error getting data at '%s'.\n", tail_path);
        return;
    }
    if (len > 0 && strcmp(tail_host, address_port) != 0)
    {
        rtree_disconnect(tail); // ! verificar erros
        tail = rtree_connect(address_port);
    }
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx)
{
    zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));
    if (state == ZOO_CONNECTED_STATE)
    {
        if (type == ZOO_CHILD_EVENT)
        {
            if (ZOK != zoo_wget_children(zh, root_path, child_watcher, w_context, children_list))
            {
                fprintf(stderr, "child_watcher: Error setting watch at '%s'.\n", root_path);
            }
            else
            {
                update_server_list(children_list);
            }
        }
    }
    free(children_list);
}

int main(int argc, char const *argv[])
{
    head = tail;
    /* Testar os argumentos de entrada */
    if (argc != 2)
    {
        printf("Uso: ./tree-client <server:port>\n");
        printf("Exemplo de uso: ./tree-client 127.0.0.1:12345\n");
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

    zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));
    if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, w_context, children_list))
    {
        fprintf(stderr, "Error setting watch at %s!\n", root_path);
    }

    getline(&line, &line_size, stdin);

    getline(&line, &line_size, stdin);

    tail = rtree_connect(argv[1]);
    if (tail == NULL)
    {
        free(line);
        exit(EXIT_FAILURE);
    }

    signal_sigint(tree_client_exit);

    struct pollfd desc_set[2];
    struct pollfd *stdin_desc = &desc_set[0];
    struct pollfd *rtree_desc = &desc_set[1];
    stdin_desc->fd = fileno(stdin); // stdin (cliente)
    stdin_desc->events = POLLIN;
    rtree_desc->fd = tail->sockfd; // ligação com o servidor
    rtree_desc->events = POLLIN;

    // esperamos por input do cliente, ou fecho do servidor
    while (poll(desc_set, 2, -1) > 0)
    {
        if (rtree_desc->revents & POLLHUP)
        {
            printf("O servidor terminou a ligação.\n");
            break;
        }
        if (rtree_desc->revents & POLLIN) // recebemos mensagem do servidor
        {
            if (available_read_bytes(rtree_desc->fd) == 0) // 0 bytes para ler
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

            int i = rtree_put(head, entry);
            if (i == -1)
            {
                printf("Erro no comando 'put'.\n");
            }
            else
            {
                printf("'%d': id da operacão 'put' para a entrada '%s'\n", i, entry->key);
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

            int i = rtree_del(head, key);
            if (i == -1)
            {
                printf("'%s': entrada inexistente\n", key);
            }
            else
            {
                printf("'%d': id da operacão 'del' para a entrada '%s'\n", i, key);
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
    int status = rtree_disconnect(tail);
    if (status != 0)
    {
        perror("tree_client");
    }
    free(line);
    exit(status);
}
