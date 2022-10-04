#include "serialization.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int keyArray_to_buffer(char **keys, char **keys_buf)
{
    int i, size;
    for (i = 0, size = sizeof(int); keys[i] != NULL; i++)
    {
        size += sizeof(int) + strlen(keys[i]) + 1;
    }
    *keys_buf = (char *)malloc(size);
    memcpy(*keys_buf, &i, sizeof(int));

    char *keys_buf_ptr = *keys_buf + sizeof(int);
    for (i = 0; keys[i] != NULL; i++)
    {
        int keylen = strlen(keys[i]) + 1;
        memcpy(keys_buf_ptr, &keylen, sizeof(int));
        keys_buf_ptr += sizeof(int);
        strcpy(keys_buf_ptr, keys[i]);
        keys_buf_ptr += keylen;
    }
    return size;
}

char **buffer_to_keyArray(char *keys_buf, int keys_buf_size)
{
    int n = *keys_buf;
    keys_buf += sizeof(int);
    char **keys = (char **)malloc((n + 1) * sizeof(char *));
    for (int i = 0; i < n; i++)
    {
        int keylen = (int)*keys_buf;
        keys_buf += sizeof(int);
        keys[i] = (char *)malloc(keylen);
        strcpy(keys[i], keys_buf);
        keys_buf += keylen;
    }
    keys[n] = NULL;
    return keys;
}

void print_str_array(char **str_array)
{
    printf("[");
    int i = 0;
    for (i = 0; str_array[i] != NULL; i++)
    {
        if (str_array[i + 1] == NULL)
        {
            printf("%s", str_array[i]);
            break;
        }
        printf("%s, ", str_array[i]);
    }
    printf("]\n");
}

/**
 * Só para testes.
 */
int main(int argc, char const *argv[])
{
    char *keys[] = {"ola", "amigos", "fcul", "sd", NULL};
    char **keys_buf = (char **)malloc(sizeof(char *));

    // INICIALIZAR
    printf("Array de chaves original: ");
    print_str_array(keys);

    // SERIALIZAR
    int keys_buf_size = keyArray_to_buffer(keys, keys_buf);
    // int keys_buf_size = (int)**keys_buf;
    printf("Tamanho do buffer serializado: %d\n", keys_buf_size);

    // DE-SERIALIZAR
    char **final_keys = buffer_to_keyArray(*keys_buf, keys_buf_size);
    printf("Array de chaves de-serializado: ");
    print_str_array(final_keys);

    // CLEANUP
    free(*keys_buf);
    free(keys_buf);
    for (int i = 0; final_keys[i] != NULL; i++)
    {
        free(final_keys[i]);
    }
    free(final_keys);
    return 0;
}
