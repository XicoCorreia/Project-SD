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

/*
 * Função que percorre a árvore "em ordem" (esquerda -> nó -> direita),
 * adicionando a **keys as chaves dos nós percorridos.
 */
void inorder_keys(struct tree_t *tree, char **keys, int *count);

/*
 * Função que percorre a árvore "em ordem" (esquerda -> nó -> direita),
 * adicionando a **values os valores dos nós percorridos.
 */
void inorder_values(struct tree_t *tree, struct data_t **values, int *count);

/*
 * Função que devolve o nó sucessor de uma dada (sub-)árvore.
 * Por outras palavras: a menor chave das chaves superiores à atual.
 * Na prática percorre recursivamente as sub-árvores esquerdas
 * da sub-árvore direita do argumento dado (direita -> esquerda * n).
 */
struct tree_t *inorder_successor(struct tree_t *tree);

/*
 * Função auxiliar usada por tree_del que devolve uma struct tree_t
 * por conveniência (chamadas recursivas). O valor a devolver em tree_del
 * é atualizado no *exit_code (0 por omissão, -1 se não encontrou a chave).
 */
struct tree_t *tree_del_aux(struct tree_t *tree, char *key, int *exit_code);

#endif
