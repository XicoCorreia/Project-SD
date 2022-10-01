#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"

struct tree_t
{
    struct tree_t *left;
    struct tree_t *right;
    char *key;
    struct data_t *value;
};

#endif
