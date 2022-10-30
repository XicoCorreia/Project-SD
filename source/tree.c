/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "tree.h"
#include "tree-private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tree_t *tree_create()
{
    struct tree_t *tree = malloc(sizeof(struct tree_t));
    // Verifica se malloc foi sucedido
    if (tree == NULL)
        return NULL;

    tree->left = tree->right = NULL;
    tree->entry = NULL;
    return tree;
}

void tree_destroy(struct tree_t *tree)
{
    // Se tree é NULL não acontece nada
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

    // No com Entry<key, value>
    if (tree->entry == NULL)
    {
        struct entry_t *entry = entry_create(key, value);
        tree->entry = entry_dup(entry);
        free(entry);
    }
    else if (strcmp(key, tree->entry->key) == 0)
    {
        // No com key já existe
        entry_replace(tree->entry, strdup(key), data_dup(value));
    }
    else if (strcmp(key, tree->entry->key) < 0)
    {
        if (tree->left == NULL)
        {
            // Cria a tree a esquerda, onde a key e o value vão ser colocados
            tree->left = tree_create();
        }
        return tree_put(tree->left, key, value);
    }
    else if (strcmp(key, tree->entry->key) > 0)
    {
        if (tree->right == NULL)
        {
            // Cria a tree direita, onde a key e o value vão ser colocados
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
        // Entry com key encontrada
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
    int ret = 0;
    tree_del_aux(tree, key, &ret);
    return ret;
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

    if (size > 0)
    {
        inorder_keys(tree, keys, &count);
    }
    keys[size] = NULL;

    return keys;
}

void **tree_get_values(struct tree_t *tree)
{
    int size = tree_size(tree);
    struct data_t **values = malloc(sizeof(struct data_t *) * (size + 1));
    int count = 0;

    if (size > 0)
    {
        inorder_values(tree, values, &count);
    }
    values[size] = NULL;

    return (void **)values;
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

void inorder_keys(struct tree_t *tree, char **keys, int *count)
{
    if (tree == NULL)
    {
        return;
    }
    else
    {
        // Primeiro arvore esquerda, depois root, por fim arvore direita
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

void inorder_values(struct tree_t *tree, struct data_t **values, int *count)
{
    if (tree == NULL)
    {
        return;
    }
    else
    {
        // Primeiro arvore esquerda, depois root, por fim arvore direita
        inorder_values(tree->left, values, count);
        values[*count] = data_dup(tree->entry->value);
        (*count)++;
        inorder_values(tree->right, values, count);
    }
}

struct tree_t *tree_del_aux(struct tree_t *tree, char *key, int *exit_code)
{
    if (tree == NULL || tree->entry == NULL)
    {
        *exit_code = -1;
        return tree;
    }
    int cmp = strcmp(key, tree->entry->key);

    if (cmp < 0)
    {
        tree->left = tree_del_aux(tree->left, key, exit_code);
    }
    else if (cmp > 0)
    {
        tree->right = tree_del_aux(tree->right, key, exit_code);
    }
    else
    {
        if (tree->left == NULL && tree->right == NULL)
        {
            tree_destroy(tree);
            tree = NULL;
        }
        else if (tree->left == NULL)
        {
            tree = tree->right;
        }
        else if (tree->right == NULL)
        {
            tree = tree->left;
        }
        else
        {
            struct tree_t *temp = inorder_successor(tree);
            entry_destroy(tree->entry);
            tree->entry = temp->entry;
            // Lidamos com o no sucessor (temp), e.g. se houver um no filho
            // a direita, este toma a posicao que o no pai tinha
            tree->right = tree_del_aux(tree->right, tree->entry->key, exit_code);
            free(temp);
        }
    }
    return tree;
}

struct tree_t *inorder_successor(struct tree_t *tree)
{
    if (tree == NULL || tree->right == NULL)
    {
        return NULL;
    }

    tree = tree->right;
    while (tree->left != NULL)
    {
        tree = tree->left;
    }
    return tree;
}
