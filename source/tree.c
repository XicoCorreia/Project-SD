/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "tree.h"
#include "tree-private.h"
#include <stdlib.h>
#include <string.h>

struct tree_t *tree_create()
{
    struct tree_t *tree = malloc(sizeof(struct tree_t));
    if (tree == NULL)
        return NULL;
    tree->left = tree->right = NULL;
    tree->entry = NULL;
    return tree;
}

void tree_destroy(struct tree_t *tree)
{
    if (tree == NULL)
        return;

    tree_destroy(tree->right);
    tree_destroy(tree->left);

    entry_destroy(tree->entry);
    free(tree);
}

int tree_put(struct tree_t *tree, char *key, struct data_t *value)
{
    if (tree == NULL)
        return -1;

    if (tree->entry == NULL)
    {
        struct entry_t *entry = entry_create(key, value);
        tree->entry = entry_dup(entry);
        free(entry);
    }
    else if (strcmp(key, tree->entry->key) == 0)
    {
        entry_replace(tree->entry, strdup(key), data_dup(value));
    }
    else if (strcmp(key, tree->entry->key) < 0)
    {
        if (tree->left == NULL)
        {
            tree->left = tree_create();
        }
        return tree_put(tree->left, key, value);
    }
    else if (strcmp(key, tree->entry->key) > 0)
    {
        if (tree->right == NULL)
        {
            tree->right = tree_create();
        }
        return tree_put(tree->right, key, value);
    }
    return 0;
}

struct data_t *tree_get(struct tree_t *tree, char *key)
{
    if (tree == NULL || tree->entry == NULL)
        return NULL;

    if (strcmp(key, tree->entry->key) == 0)
    {
        return data_dup(tree->entry->value);
    }
    else if (strcmp(key, tree->entry->key) < 0)
    {
        return tree_get(tree->left, key);
    }
    else
    {
        return tree_get(tree->right, key);
    }
}

int tree_del(struct tree_t *tree, char *key)
{
    int size = tree_size(tree);
    tree_del_aux(tree, key);
    return size - tree_size(tree) - 1;
}

int tree_size(struct tree_t *tree)
{
    if (tree == NULL || tree->entry == NULL)
        return 0;
    else
        return (tree_size(tree->left) + 1 + tree_size(tree->right));
}

int tree_height(struct tree_t *tree)
{
    if (tree == NULL)
        return 0;
    else
    {
        int l_height = tree_height(tree->left);
        int r_height = tree_height(tree->right);

        if (l_height >= r_height)
            return l_height + 1;
        else
            return r_height + 1;
    }
}

char **tree_get_keys(struct tree_t *tree)
{
    int size = tree_size(tree);
    char **keys = malloc(sizeof(char *) * (size + 1));
    int count = 0;

    inorder_keys(tree, keys, &count);
    keys[size] = NULL;

    return keys;
}

void **tree_get_values(struct tree_t *tree)
{
    int size = tree_size(tree);
    void **values = malloc(sizeof(void *) * (size + 1));
    int count = 0;

    inorder_values(tree, values, &count);
    values[size] = NULL;

    return values;
}

void tree_free_keys(char **keys)
{
    int count = 0;
    while (keys[count] != NULL)
    {
        free(keys[count]);
        count++;
    }
    free(keys);
}

void tree_free_values(void **values)
{
    int count = 0;
    while (values[count] != NULL)
    {
        free(values[count]);
        count++;
    }
    free(values);
}
