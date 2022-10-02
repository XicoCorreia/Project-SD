#include "serialization.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int keyArray_to_buffer(char **keys, char **keys_buf)
{
    int i;
    int size = 0;
    for (i = 0; keys[i] != NULL; i++)
    {
        size += strlen(keys[i]) + 1; // incluir '\0' de fim de string
    }
    *keys_buf = (char *)malloc(sizeof(int) + size);
    memcpy(*keys_buf, &i, sizeof(int)); // nº de elementos no inicio do buf
    memcpy(*keys_buf + sizeof(int), *keys, size);
    return i;
}

char **buffer_to_keyArray(char *keys_buf, int keys_buf_size)
{
    char **keys = (char **)malloc(keys_buf_size * sizeof(char *));
    int size = 0;
    for (int i = 0; i < keys_buf_size; i++)
    {
        size = strlen(keys_buf) + 1;
        keys[i] = (char *)malloc(strlen(keys_buf) + 1);
        strcpy(keys[i], keys_buf);
        keys_buf += size;
    }
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
    char **final_keys = buffer_to_keyArray(*(keys_buf) + sizeof(int), keys_buf_size);
    printf("Array de chaves de-serializado: ");
    print_str_array(final_keys);
    return 0;
}
