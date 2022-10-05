/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
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

struct tree_t *tree_del_aux(struct tree_t *tree, char *key)
{
    if (tree == NULL)
        return NULL;
    if (tree->entry == NULL)
        return tree;

    int cmp = strcmp(key, tree->entry->key);

    if (cmp < 0)
    {
        tree->left = tree_del_aux(tree->left, key);
    }
    else if (cmp > 0)
    {
        tree->right = tree_del_aux(tree->right, key);
    }
    else
    {
        struct tree_t *temp;
        if (tree->left == NULL)
        {
            temp = tree->right;
            return temp;
        }
        else if (tree->right == NULL)
        {
            temp = tree->left;
            return temp;
        }

        temp = inorder_successor(tree->right);
        tree->entry = temp->entry;
        tree->right = tree_del_aux(tree->right, temp->entry->key);
    }
    return tree;
}

struct tree_t *inorder_successor(struct tree_t *tree)
{
    if (tree == NULL)
    {
        return NULL;
    }
    while (tree->left != NULL)
    {
        tree = tree->left;
    }
    return tree;
}