#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
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
    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    EntryT entry_msg = ENTRY_T__INIT;
    entry_msg.key = entry->key;
    entry_msg.value.len = entry->value->datasize;
    entry_msg.value.data = malloc(entry->value->datasize);
    memcpy(entry_msg.value.data, entry->value->data, entry->value->datasize);

    msg->data.len = entry_t__pack(&entry_msg, msg->data.data);

    if ((msg = network_send_receive(rtree, msg)) == NULL)
    {
        return -1;
    }
    int ret = *msg->data.data;
    return msg->opcode == MESSAGE_T__OPCODE__OP_ERROR ? -1 : ret;
}

struct data_t *rtree_get(struct rtree_t *rtree, char *key)
{
    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GET;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg->data.len = strlen(key) + 1;
    msg->data.data = (uint8_t *)key;
    if ((msg = network_send_receive(rtree, msg)) == NULL)
    {
        return NULL;
    }
    if (msg->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return NULL;
    }
    return (struct data_t *)msg->data.data;
}

int rtree_del(struct rtree_t *rtree, char *key)
{
    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg->data.len = strlen(key) + 1;
    msg->data.data = (uint8_t *)key;
    if ((msg = network_send_receive(rtree, msg)) == NULL)
    {
        return -1;
    }
    int ret = *msg->data.data;
    return msg->opcode == MESSAGE_T__OPCODE__OP_ERROR ? -1 : ret;
}

int rtree_size(struct rtree_t *rtree)
{
    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->data.len = 0;
    msg->data.data = NULL;
    if ((msg = network_send_receive(rtree, msg)) == NULL)
    {
        return -1;
    }
    int ret = *msg->data.data;
    return msg->opcode == MESSAGE_T__OPCODE__OP_ERROR ? -1 : ret;
}

int rtree_height(struct rtree_t *rtree)
{
    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->data.len = 0;
    msg->data.data = NULL;
    if ((msg = network_send_receive(rtree, msg)) == NULL)
    {
        return -1;
    }
    int ret = *msg->data.data;
    return msg->opcode == MESSAGE_T__OPCODE__OP_ERROR ? -1 : ret;
}

char **rtree_get_keys(struct rtree_t *rtree)
{
    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->data.len = 0;
    msg->data.data = NULL;
    if ((msg = network_send_receive(rtree, msg)) == NULL)
    {
        return NULL;
    }
    if (msg->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return NULL;
    }
    return (char **)msg->data.data;
}

void **rtree_get_values(struct rtree_t *rtree)
{
    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->data.len = 0;
    msg->data.data = NULL;
    if ((msg = network_send_receive(rtree, msg)) == NULL)
    {
        return NULL;
    }
    if (msg->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        return NULL;
    }
    return (void **)msg->data.data;
}
