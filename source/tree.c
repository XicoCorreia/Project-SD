#include "tree.h"
#include "tree-private.h"
#include <string.h>

struct tree_t *tree_create()
{
}

void tree_destroy(struct tree_t *tree)
{
}

int tree_put(struct tree_t *tree, char *key, struct data_t *value)
{
    if (strcmp(key, tree->key) == 0)
    {
        // IGUAIS
    }
    else if (strcmp(key, tree->key) <= 0)
    {
        // return tree_put(tree->left, key, value);
    }
    // ...
}

struct data_t *tree_get(struct tree_t *tree, char *key)
{
}

int tree_del(struct tree_t *tree, char *key)
{
}

int tree_size(struct tree_t *tree)
{
}

int tree_height(struct tree_t *tree)
{
}

char **tree_get_keys(struct tree_t *tree)
{
}

void **tree_get_values(struct tree_t *tree)
{
}

void tree_free_keys(char **keys)
{
}

void tree_free_values(void **values)
{
}
