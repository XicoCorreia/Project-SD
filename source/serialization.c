/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "serialization.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int keyArray_to_buffer(char **keys, char **keys_buf)
{
    int keys_buf_size = 0;
    for (int i = 0; keys[i] != NULL; i++)
    {
        keys_buf_size += sizeof(int) + strlen(keys[i]) + 1;
    }
    *keys_buf = malloc(keys_buf_size);

    keys_buf_size = 0;
    for (int i = 0; keys[i] != NULL; i++)
    {
        int keylen = strlen(keys[i]) + 1;
        memcpy(*keys_buf + keys_buf_size, &keylen, sizeof(int));
        strcpy(*keys_buf + keys_buf_size + sizeof(int), keys[i]);
        keys_buf_size += sizeof(int) + keylen;
    }
    return keys_buf_size;
}

char **buffer_to_keyArray(char *keys_buf, int keys_buf_size)
{
    if (keys_buf == NULL || keys_buf_size <= 0)
        return NULL;

    int n = 0;
    int offset = 0;
    char **keys;

    while (offset < keys_buf_size)
    {
        offset += *(keys_buf + offset) + sizeof(int);
        n++;
    }

    keys = malloc((n + 1) * sizeof(char *));
    if (keys == NULL)
        return NULL;

    offset = 0;
    for (int i = 0; i < n; i++)
    {
        keys[i] = strdup(keys_buf + offset + sizeof(int));
        offset += *(keys_buf + offset) + sizeof(int);
    }
    keys[n] = NULL;

    return keys;
}
