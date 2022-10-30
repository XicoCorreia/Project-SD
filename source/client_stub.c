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
    return 0;
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
    return 0;
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
    keys = result->keys;
    free(result);
    if (keys == NULL) // sem erro mas 0 chaves na tree
    {
        keys = calloc(1, sizeof(char *));
        if (keys == NULL)
        {
            perror("rtree_get_keys");
        }
    }
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
    values = (struct data_t **)result->values;
    free(result);
    if (values == NULL) // sem erro mas 0 valores na tree
    {
        values = calloc(1, sizeof(void *));
        if (values == NULL)
        {
            perror("rtree_get_values");
        }
    }
    return (void **)values;
}
