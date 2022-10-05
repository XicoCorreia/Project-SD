#include "serialization.h"
#include <stdio.h>
#include <stdlib.h>

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
    char **keys_buf = malloc(sizeof(char *));

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
