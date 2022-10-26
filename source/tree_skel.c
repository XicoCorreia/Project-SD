#include "tree_skel.h"
#include "entry.h"
#include "string.h"
#include "tree-private.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>

struct tree_t *tree;

int tree_skel_init()
{
    tree = tree_create();
    if (tree == NULL)
    {
        return -1;
    }
    return 0;
}

void tree_skel_destroy()
{
    tree_destroy(tree);
}

int invoke(MessageT *msg)
{
    // TODO

    switch (msg->opcode)
    {
    case MESSAGE_T__OPCODE__OP_SIZE:
        int size = tree_size(tree);
        msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->data.len = sizeof(int);
        msg->data.data = (int *)malloc(sizeof(int));
        *msg->data.data = size;
        break;
    case MESSAGE_T__OPCODE__OP_HEIGHT:
        int height = tree_height(tree);
        msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->data.len = sizeof(int);
        msg->data.data = (int *)malloc(sizeof(int));
        *msg->data.data = height;
        break;
    case MESSAGE_T__OPCODE__OP_DEL:
        if (tree_del(tree, (char *)msg->data.data) < 0)
        {
            // tratar como OP_BAD ou ignorar?
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        }
        free(msg->data.data);
        msg->data.len = 0;
        msg->data.data = NULL;
        break;
    case MESSAGE_T__OPCODE__OP_GET:
        struct data_t *value = tree_get(tree, (char *)msg->data.data);
        free(msg->data.data);
        if (value == NULL)
        {
            msg->data.data = NULL;
            msg->data.len = 0;
        }
        else
        {
            msg->data.data = malloc(value->datasize);
            memcpy(msg->data.data, value->data, value->datasize);
            msg->data.len = value->datasize;
        }
        msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        break;
    case MESSAGE_T__OPCODE__OP_PUT:
        struct entry_t *entry = msg->data.data;
        if (tree_put(tree, entry->key, entry->value) < 0)
        {
            // tratar como OP_BAD ou ignorar?
        }
        else
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        }
        free(msg->data.data);
        msg->data.len = 0;
        msg->data.data = NULL;
        break;
    case MESSAGE_T__OPCODE__OP_GETKEYS:
        KeysT keys = KEYS_T__INIT;
        keys.keys = tree_get_keys(tree);

        int i;
        for (i = 0; keys.keys[i] != NULL; i++)
            ;
        keys.n_keys = i;

        msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        free(msg->data.data);
        msg->data.len = keys_t__get_packed_size(&keys);
        msg->data.data = malloc(msg->data.len);
        keys_t__pack(&keys, msg->data.data);
        break;
    case MESSAGE_T__OPCODE__OP_GETVALUES:
        ValuesT values = VALUES_T__INIT;
        values.values->data = tree_get_values(tree);

        int i;
        for (i = 0; values.values[i].data != NULL; i++)
        {
            // ! Os dados são void*: não podemos só strlen...
            values.values[i].len = strlen((char *)values.values[i].data);
        }
        values.n_values = i;

        msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;
        free(msg->data.data);
        msg->data.len = values_t__get_packed_size(&keys);
        msg->data.data = malloc(msg->data.len);
        keys_t__pack(&keys, msg->data.data);
        break;
    default:
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        free(msg->data.data);
        msg->data.len = 0;
        msg->data.data = NULL;
        return -1;
    }
    return 0;
}
