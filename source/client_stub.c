/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rtree_t *rtree_connect(const char *address_port)
{
    struct rtree_t *rtree;
    char *address;
    u_int16_t port;

    char *sep = strchr(address_port, ':');
    if (sep == NULL)
    {
        return NULL;
    }

    address = strndup(address_port, sep - address_port);
    port = atoi(sep + 1);
    if (port <= 0)
    {
        return NULL;
    }
    rtree = malloc(sizeof(struct rtree_t));
    rtree->address = address;
    rtree->port = port;
    if (network_connect(rtree) == -1)
    {
        free(rtree->address);
        free(rtree);
        return NULL;
    }
    return rtree;
}

int rtree_disconnect(struct rtree_t *rtree)
{
    int ret = network_close(rtree);
    free(rtree->address);
    free(rtree);
    return ret;
}

int rtree_put(struct rtree_t *rtree, struct entry_t *entry)
{
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    EntryT entry_msg = ENTRY_T__INIT;
    entry_msg.key = entry->key;
    entry_msg.value.len = entry->value->datasize;
    entry_msg.value.data = malloc(entry->value->datasize);
    memcpy(entry_msg.value.data, entry->value->data, entry->value->datasize);

    msg.data.len = entry_t__get_packed_size(&entry_msg);
    uint8_t *data = malloc(entry_t__get_packed_size(&entry_msg));
    msg.data.data = data;
    entry_t__pack(&entry_msg, msg.data.data);

    if (network_send_receive(rtree, &msg) == NULL)
    {
        free(data);
        free(entry_msg.value.data);
        return -1;
    }

    free(data);
    free(entry_msg.value.data);

    if (msg.opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return -1;
    }
    int ret = *((int *)msg.data.data);
    free(msg.data.data);
    return ret;
}

struct data_t *rtree_get(struct rtree_t *rtree, char *key)
{
    MessageT msg = MESSAGE_T__INIT;
    DataT *result;
    struct data_t *data;

    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg.data.len = strlen(key) + 1;
    msg.data.data = (uint8_t *)key;
    if (network_send_receive(rtree, &msg) == NULL)
    {
        return NULL;
    }
    if (msg.opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return NULL;
    }
    result = data_t__unpack(NULL, msg.data.len, msg.data.data);
    free(msg.data.data);
    data = malloc(sizeof(struct data_t));
    data->datasize = result->data.len;
    data->data = result->data.data;
    free(result);
    return data;
}

int rtree_del(struct rtree_t *rtree, char *key)
{
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg.data.len = strlen(key) + 1;
    msg.data.data = (uint8_t *)key;
    if (network_send_receive(rtree, &msg) == NULL)
    {
        return -1;
    }
    if (msg.opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return -1;
    }

    int ret = *((int *)msg.data.data);
    free(msg.data.data);
    return ret;
}

int rtree_size(struct rtree_t *rtree)
{
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg.data.len = 0;
    msg.data.data = NULL;
    if (network_send_receive(rtree, &msg) == NULL)
    {
        return -1;
    }
    if (msg.opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return -1;
    }
    int ret = *((int *)msg.data.data);
    free(msg.data.data);
    return ret;
}

int rtree_height(struct rtree_t *rtree)
{
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg.data.len = 0;
    msg.data.data = NULL;
    if (network_send_receive(rtree, &msg) == NULL)
    {
        return -1;
    }
    if (msg.opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return -1;
    }
    int ret = *((int *)msg.data.data);
    free(msg.data.data);
    return ret;
}

char **rtree_get_keys(struct rtree_t *rtree)
{
    MessageT msg = MESSAGE_T__INIT;
    KeysT *result;
    char **keys;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg.data.len = 0;
    msg.data.data = NULL;
    if (network_send_receive(rtree, &msg) == NULL)
    {
        return NULL;
    }
    if (msg.opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return NULL;
    }
    result = keys_t__unpack(NULL, msg.data.len, msg.data.data);
    free(msg.data.data);
    keys = malloc(sizeof(char *) * (result->n_keys + 1)); // +1 para NULL

    if (keys == NULL)
    {
        perror("rtree_get_keys");
        free(result);
        return NULL;
    }
    memcpy(keys, result->keys, sizeof(char *) * result->n_keys);
    keys[result->n_keys] = NULL;
    free(result->keys);
    free(result);
    return keys;
}

void **rtree_get_values(struct rtree_t *rtree)
{
    MessageT msg = MESSAGE_T__INIT;
    ValuesT *result;
    struct data_t **values;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg.data.len = 0;
    msg.data.data = NULL;
    if (network_send_receive(rtree, &msg) == NULL)
    {
        return NULL;
    }
    if (msg.opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return NULL;
    }
    result = values_t__unpack(NULL, msg.data.len, msg.data.data);
    free(msg.data.data);
    values = malloc(sizeof(struct data_t *) * (result->n_values + 1)); // +1 para NULL

    if (values == NULL)
    {
        perror("rtree_get_keys");
        free(result);
        return NULL;
    }

    for (int i = 0; i < result->n_values; i++)
    {
        values[i] = data_create2(result->values[i].len, result->values[i].data);
    }
    values[result->n_values] = NULL;
    free(result->values);
    free(result);
    return (void **)values;
}

void rtree_free_keys(char **keys)
{
    int count = 0;
    while (keys[count] != NULL)
    {
        free(keys[count]);
        count++;
    }
    free(keys);
}

void rtree_free_values(void **values)
{
    struct data_t **values_ptr = (struct data_t **)values;
    int count = 0;
    while (values_ptr[count] != NULL)
    {
        free(values_ptr[count]->data);
        free(values_ptr[count]);
        count++;
    }
    free(values_ptr);
}

int rtree_verify(struct rtree_t *rtree, int op_n) 
{
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_VERIFY;                 
    msg.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg.data.len = strlen(op_n) + 1;
    msg.data.data = (uint8_t *)op_n;
    if (network_send_receive(rtree, &msg) == NULL)
    {
        return -1;
    }
    if (msg.opcode == MESSAGE_T__OPCODE__OP_ERROR)         
    {
        return -1;
    }

    int ret = *((int *)msg.data.data);
    free(msg.data.data);
    return ret;
}
