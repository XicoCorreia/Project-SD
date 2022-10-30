/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
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
    int status;
    char *key;
    struct data_t *value;

    switch (msg->opcode)
    {
    case MESSAGE_T__OPCODE__OP_SIZE:
        int size = tree_size(tree);
        msg->data.len = sizeof(int);
        msg->data.data = malloc(sizeof(int));
        *((int *)msg->data.data) = size;
        msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        break;

    case MESSAGE_T__OPCODE__OP_HEIGHT:
        int height = tree_height(tree);
        msg->data.len = sizeof(int);
        msg->data.data = malloc(sizeof(int));
        *((int *)msg->data.data) = height;
        msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        break;

    case MESSAGE_T__OPCODE__OP_DEL:
        key = (char *)msg->data.data;
        status = tree_del(tree, key);

        if (status < 0)
        {
            printf("del: Chave '%s' nao encontrada.\n", key);
        }

        free(msg->data.data);
        msg->data.len = 0;
        msg->data.data = NULL;
        msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        break;

    case MESSAGE_T__OPCODE__OP_GET:
        key = (char *)msg->data.data;
        value = tree_get(tree, key);

        if (value == NULL)
        {
            printf("get: Chave '%s' nao encontrada.\n", key);
            value = calloc(1, sizeof(struct data_t)); // size=0, data=NULL
        }

        free(msg->data.data);
        DataT data = DATA_T__INIT;
        data.data.len = value->datasize;
        data.data.data = value->data;
        msg->data.len = data_t__get_packed_size(&data);
        msg->data.data = malloc(msg->data.len);
        data_t__pack(&data, msg->data.data);
        data_destroy(value);
        msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        break;

    case MESSAGE_T__OPCODE__OP_PUT:
        EntryT *entry = entry_t__unpack(NULL, msg->data.len, msg->data.data);
        value = data_create2(entry->value.len, entry->value.data);
        status = tree_put(tree, entry->key, value);
        free(msg->data.data);
        free(value);
        entry_t__free_unpacked(entry, NULL);

        if (status < 0)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
        }

        msg->data.len = 0;
        msg->data.data = NULL;
        msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        break;

    case MESSAGE_T__OPCODE__OP_GETKEYS:
        KeysT keys = KEYS_T__INIT;
        keys.keys = tree_get_keys(tree);

        if (keys.keys == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
        }

        keys.n_keys = tree_size(tree);
        msg->data.len = keys_t__get_packed_size(&keys);
        msg->data.data = malloc(msg->data.len);
        keys_t__pack(&keys, msg->data.data);
        tree_free_keys(keys.keys);
        msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        break;

    case MESSAGE_T__OPCODE__OP_GETVALUES:
        struct data_t **data_arr = (struct data_t **)tree_get_values(tree);
        if (data_arr == NULL)
        {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
        }
        ValuesT *values = malloc(sizeof(ValuesT)); // free_unpacked requer memória dinâmica
        values_t__init(values);
        values->n_values = tree_size(tree);
        values->values = malloc(sizeof(ProtobufCBinaryData) * values->n_values);
        for (int i = 0; i < values->n_values; i++)
        {
            values->values[i].len = data_arr[i]->datasize;
            values->values[i].data = data_arr[i]->data;
            free(data_arr[i]);
        }
        free(data_arr);
        msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;

        msg->data.len = values_t__get_packed_size(values);
        msg->data.data = malloc(msg->data.len);
        values_t__pack(values, msg->data.data);
        values_t__free_unpacked(values, NULL);
        break;

    default:
        msg->opcode = MESSAGE_T__OPCODE__OP_BAD;
        msg->c_type = MESSAGE_T__C_TYPE__CT_BAD;
        break;
    }

    if (msg->opcode == MESSAGE_T__OPCODE__OP_BAD || msg->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        free(msg->data.data);
        msg->data.len = 0;
        msg->data.data = NULL;
        return -1;
    }
    return 0;
}
