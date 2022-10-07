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
    if (keys == NULL || keys_buf == NULL)
        return -1;

    int i;
    int keys_buf_size = sizeof(int); // num. de elementos no inicio do buffer
    for (i = 0; keys[i] != NULL; i++)
    {
        keys_buf_size += sizeof(int) + strlen(keys[i]);
    }

    *keys_buf = malloc(keys_buf_size);
    if (*keys_buf == NULL)
        return -1;

    memcpy(*keys_buf, &i, sizeof(int));
    keys_buf_size = sizeof(int);
    for (i = 0; keys[i] != NULL; i++)
    {
        int keylen = strlen(keys[i]);
        memcpy(*keys_buf + keys_buf_size, &keylen, sizeof(int));
        memcpy(*keys_buf + keys_buf_size + sizeof(int), keys[i], keylen);
        keys_buf_size += sizeof(int) + keylen;
    }
    return keys_buf_size;
}

char **buffer_to_keyArray(char *keys_buf, int keys_buf_size)
{
    if (keys_buf == NULL || keys_buf_size <= 0)
        return NULL;

    int n = (int)*keys_buf;
    int offset = sizeof(int);

    char **keys = malloc((n + 1) * sizeof(char *));
    if (keys == NULL)
        return NULL;

    keys[n] = NULL;
    for (int i = 0; i < n; i++)
    {
        int keylen = (int)*(keys_buf + offset);
        keys[i] = malloc(keylen + 1);
        if (keys[i] == NULL)
            return NULL;

        memcpy(keys[i], keys_buf + offset + sizeof(int), keylen);
        keys[i][keylen] = '\0';
        offset += keylen + sizeof(int);
    }

    return keys;
}
