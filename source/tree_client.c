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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t LINE_SIZE = 2048;
struct rtree_t *rtree;

int main(int argc, char const *argv[])
{
    /* Testar os argumentos de entrada */
    if (argc != 2)
    {
        printf("Uso: ./tree-client <server:port>\n");
        printf("Exemplo de uso: ./tree-client 127.0.0.1:12345\n");
        exit(EXIT_FAILURE);
    }
    rtree = rtree_connect(argv[1]);
    if (rtree == NULL)
    {
        exit(EXIT_FAILURE);
    }

    signal_sigint(tree_client_exit);
    while (true)
    {
        char str[LINE_SIZE];
        fgets(str, LINE_SIZE, stdin);
        str[strlen(str) - 1] = '\0';

        char *token = strtok(str, " ");
        if (token == NULL)
        {
            printf("Comando nao reconhecido. Tente novamente.\n");
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

            int i = rtree_put(rtree, entry);
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

            struct data_t *data = rtree_get(rtree, key);
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

            int i = rtree_del(rtree, key);
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

            int i = rtree_verify(rtree, n_op);
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
            int size = rtree_size(rtree);
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
            int height = rtree_height(rtree);
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
            char **keys = rtree_get_keys(rtree);
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
            struct data_t **values = (struct data_t **)rtree_get_values(rtree);
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
    int status = rtree_disconnect(rtree);
    if (status != 0)
    {
        perror("tree_client");
    }
    exit(status);
}
