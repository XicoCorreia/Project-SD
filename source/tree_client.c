#include <client_stub-private.h>
#include <client_stub.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    struct rtree_t *rtree = rtree_connect(argv[1]); // ! Verificar argumentos

    while (true)
    {
        char str[50];
        fgets(str, sizeof(str), stdin);
        str[strlen(str) - 1] = '\0';

        char *token = strtok(str, " ");

        if (strcmp(token, "put") == 0)
        {

            token = strtok(NULL, " ");
            char *key = strdup(token);
            char *value = malloc(sizeof(char) * 100); // ! Tem tamanho definido

            token = strtok(NULL, " ");
            while (token != NULL)
            {
                strcat(value, token);
                token = strtok(NULL, " ");
                if (token != NULL)
                    strcat(value, " ");
            }

            struct data_t *data = data_create2(strlen(value), value);
            struct entry_t *entry = entry_create(key, data);

            int i = rtree_put(rtree, entry);
            if (i == -1)
            {
                perror("Erro no comando put\n");
                return -1;
            }
        }
        else if (strcmp(token, "get") == 0)
        {
            char *key = strtok(NULL, " ");
            struct data_t *data = rtree_get(rtree, key);
            if (data == NULL)
            {
                perror("Erro no comando get\n");
                return -1;
            }
        }
        else if (strcmp(token, "del") == 0)
        {
            char *key = strtok(NULL, " ");
            int i = rtree_del(rtree, key);
            if (i == -1)
            {
                perror("Erro no comando del\n");
                return -1;
            }
        }
        else if (strcmp(token, "size") == 0)
        {
            int size = rtree_size(rtree);
            if (size == -1)
            {
                perror("Erro no comando size\n");
                return -1;
            }
            else
                printf("%d\n", size);
        }
        else if (strcmp(token, "height") == 0)
        {
            int height = rtree_height(rtree);
            if (height == -1)
            {
                perror("Erro no comando height\n");
                return -1;
            }
            else
                printf("%d\n", height);
        }
        else if (strcmp(token, "getkeys") == 0)
        {
            char **keys = rtree_get_keys(rtree);
            if (keys == NULL)
            {
                perror("(tree_client) rtree_get_keys");
                continue;
            }
            if (keys[0] != NULL)
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
        }
        else if (strcmp(token, "getvalues") == 0)
        {
            struct data_t **values = (struct data_t **)rtree_get_values(rtree);
            if (values == NULL)
            {
                perror("(tree_client) rtree_get_values");
                continue;
            }
            if (values[0] != NULL)
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
        }
        else if (strcmp(token, "quit") == 0)
        {
            int i = rtree_disconnect(rtree); // ? Colocar o disconnect fora do while
            if (i == -1)
            {
                perror("Erro no comando quit\n");
                return -1;
            }
            break;
        }
    }
    return 0;
}