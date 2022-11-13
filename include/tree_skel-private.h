#ifndef _TREE_SKEL_PRIVATE_H
#define _TREE_SKEL_PRIVATE_H

#include "tree_skel.h"

/* Acrescenta um pedido (de escrita) à fila de pedidos.
 * O pedido será processado por uma thread secundária.
 */
void queue_add_request(request_t *request);

/* Obtém o primeiro pedido (de escrita) da fila de pedidos.
 * Se a fila estiver vazia, espera até que um pedido
 * esteja disponível para ser processado.
 */
request_t *queue_get_request();

/* Cria um pedido, reservando a memória necessária para o mesmo,
 * e copia os argumentos para a estrutura criada.
 */
request_t *create_request(int op_n, int op, char *key, struct data_t *data);

#endif
