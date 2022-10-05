#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct data_t *data_create(int size)
{
    struct data_t *new_data = malloc(sizeof(struct data_t));
    if (new_data == NULL)
    {
        perror("data_create");
        exit(1);
    }
    new_data->datasize = size;
    new_data->data = malloc(size);
    if (new_data->data == NULL)
    {
        perror("data_create");
        exit(1);
    }
    return new_data;
}

struct data_t *data_create2(int size, void *data)
{
    struct data_t *new_data = malloc(sizeof(struct data_t));
    if (new_data == NULL)
    {
        perror("data_create2");
        exit(1);
    }
    new_data->datasize = size;
    new_data->data = data;
    return new_data;
}

void data_destroy(struct data_t *data)
{
    free(data->data);
    free(data);
}

struct data_t *data_dup(struct data_t *data)
{
    struct data_t *new_data = data_create(data->datasize);
    memcpy(new_data->data, data->data, data->datasize);
    return new_data;
}

void data_replace(struct data_t *data, int new_size, void *new_data)
{
    free(data->data);
    data->data = new_data;
    data->datasize = new_size;
}
