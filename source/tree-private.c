#include "tree-private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void inorder_keys(struct tree_t *tree, char **keys, int *count)
{
    if (tree == NULL)
    {
        return;
    }
    else
    {
        inorder_keys(tree->left, keys, count);
        keys[*count] = malloc(strlen(tree->entry->key) + 1);
        if (keys[*count] == NULL)
        {
            perror("inorder_keys");
            exit(1);
        }
        strcpy(keys[*count], tree->entry->key);
        (*count)++;
        inorder_keys(tree->right, keys, count);
    }
}

void inorder_values(struct tree_t *tree, void **values, int *count)
{
    if (tree == NULL)
    {
        return;
    }
    else
    {
        inorder_values(tree->left, values, count);
        int datasize = tree->entry->value->datasize;
        values[*count] = malloc(datasize);
        memcpy(values[*count], tree->entry->value->data, datasize);
        (*count)++;
        inorder_values(tree->right, values, count);
    }
}