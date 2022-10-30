#include <client_stub-private.h>
#include <client_stub.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t MAX_KEY = 2048;
size_t MAX_VAL = 32768;

int main(int argc, char const *argv[])
{
    /* Testar os argumentos de entrada */
    if (argc != 2)
    {
        printf("Uso: ./tree-client <server:port>\n");
        printf("Exemplo de uso: ./tree-client 127.0.0.1:12345\n");
        exit(EXIT_FAILURE);
    }
    struct rtree_t *rtree = rtree_connect(argv[1]);
    if (rtree == NULL)
    {
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        char str[MAX_KEY];
        fgets(str, MAX_KEY, stdin);
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
                printf("Nao existe entrada associada a chave '%s'.\n", key);
            }
            else
            {
                printf("'%s': ", key);
                char *value = (char *)data->data;
                if (value[data->datasize - 1] == '\0')
                {
                    printf("'%s'\n", value); // imprime dados null-terminated
                }
                else
                {
                    printf("Tipo desconhecido (%d bytes)\n", data->datasize);
                }
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
                printf("Erro no comando 'del'.\n");
            }
            else
            {
                printf("Removida a entrada '%s'\n", key);
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
                printf("[%s", keys[0]);
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
                printf("[%s", (char *)values[0]->data);
                for (int i = 1; values[i] != NULL; i++)
                {
                    printf(", %s", (char *)values[i]->data); // ! assume-se string
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

    int status = rtree_disconnect(rtree);
    if (status != 0)
    {
        perror("tree_client");
    }
    return status;
}
