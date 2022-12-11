#ifndef _TREE_CLIENT_PRIVATE_H
#define _TREE_CLIENT_PRIVATE_H

#include "data.h"
#include "zookeeper/zookeeper.h"

typedef struct String_vector zoo_string;

/**
 * Função wrapper que usa strcmp
 * para comparar termos no qsort.
 */
int compare_fn(const void *a, const void *b);

/**
 * Função que verifica repetidamente
 * se uma operação já foi propagada
 * pela cadeia de servidores.
 */
int tail_verify(int op_n);

/**
 * Função chamada por child_watcher que atualiza os servidores
 * head e tail para este cliente.
 */
void update_head_tail(zoo_string *children_list);

/**
 * Função que imprime o valor associado a uma chave.
 * Se o valor não for passível de imprimir como string,
 * imprime-se "desconhecido" e o tamanho do valor.
 */
void print_value(struct data_t *data);

/**
 * Função que encerra o programa tree_client,
 * fechando a ligação com o servidor remoto e
 * libertando a memória usada.
 */
void tree_client_exit();

#endif
