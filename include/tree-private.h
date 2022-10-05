#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "entry.h"
#include "tree.h"

struct tree_t
{
    struct tree_t *left;
    struct tree_t *right;
    struct entry_t *entry;
};

void inorder_keys(struct tree_t *tree, char **keys, int *count);

void inorder_values(struct tree_t *tree, void **values, int *count);

#endif
