#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include <poll.h>

/* Obtém o indice mais alto a ser utilizado no array
 */
int get_new_max(int cur_max, struct pollfd **desc_set);

/* Obtém o próximo indice disponivel no array
 */
int get_next_index(struct pollfd **desc_set);

/* Fecha a socket e atualiza o valor do array na posição dada
 */
void close_socket(int n, struct pollfd **set);

#endif