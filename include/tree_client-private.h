#ifndef _TREE_CLIENT_PRIVATE_H
#define _TREE_CLIENT_PRIVATE_H

#include "data.h"

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
void sigint_handler();

#endif
