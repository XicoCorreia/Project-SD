#include "data.h"
#include <stdlib.h>

struct data_t *data_create(int size)
{
    struct data_t *data_s = malloc(sizeof(struct data_t));
    data_s->datasize = size;
    data_s->data = malloc(size);
    return data_s;
}

struct data_t *data_create2(int size, void *data)
{
    struct data_t *data_s = malloc(sizeof(struct data_t));
    data_s->datasize = size;
    data_s->data = data;
    return data_s;
}

void data_destroy(struct data_t *data)
{
}

struct data_t *data_dup(struct data_t *data)
{
}

void data_replace(struct data_t *data, int new_size, void *new_data)
{
}
