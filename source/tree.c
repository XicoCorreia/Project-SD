#include "tree.h"
#include "tree-private.h"
#include <stdlib.h>
#include <string.h>

struct tree_t *tree_create()
{
    struct tree_t *tree = malloc(sizeof(struct tree_t));
    if (tree == NULL)
        return NULL;
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
    {
        tree = tree_create();
        tree->entry = entry_create(key, value);
        tree->left = tree->right = NULL;
    }
    else if (strcmp(key, tree->entry->key) == 0)
    {
        entry_replace(tree->entry, key, value);
    }
    else if (strcmp(key, tree->entry->key) <= 0)
    {
        return tree_put(tree->left, key, value);
    }
    else if (strcmp(key, tree->entry->key) <= 0)
    {
        return tree_put(tree->right, key, value);
    }
    if (tree == NULL)
        return -1;
    return 0;
}

struct data_t *tree_get(struct tree_t *tree, char *key)
{
    struct data_t *value;
    if (strcmp(key, tree->entry->key) == 0)
    {
        value = data_dup(tree->entry->value);
    }
    else if (strcmp(key, tree->entry->key) <= 0)
    {
        return tree_get(tree->left, key);
    }
    else if (strcmp(key, tree->entry->key) <= 0)
    {
        return tree_get(tree->right, key);
    }
    else
    {
        return NULL;
    }

    return value;
}

int tree_del(struct tree_t *tree, char *key)
{
    // TODO
    return 1;
}

int tree_size(struct tree_t *tree)
{
    if (tree == NULL)
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
    char **keys = malloc(sizeof(char *) * tree_size(tree) + 1);
    int count = 0;

    inorder_keys(tree, keys, &count);
    keys[tree_height(tree)] = NULL;

    return keys;
}

void **tree_get_values(struct tree_t *tree)
{
    void **values = malloc(sizeof(void *) * tree_size(tree) + 1);
    int count = 0;

    inorder_values(tree, values, &count);
    values[tree_height(tree)] = NULL;

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
