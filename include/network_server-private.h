#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include <poll.h>

/* Esta função fecha o socket no índice do array de pollfd dado.
 * O valor do descritor na estrutura é colocado a -1 após fechar o socket.
 */
void close_client_socket(struct pollfd *set, int index);

/* (Re-)aloca memória para crescer (estritamente) o array de pollfd indicado,
 * colocando os campos fd dos novos elementos a -1.
 * Se o pointer dado em argumento for NULL, a função aloca memória de raíz.
 * A função devolve NULL caso:
 * - o número de elementos desejado seja negativo;
 * - o número de elementos desejado seja inferior ao número atual de elementos;
 * - ocorra um erro na alocação de memória.
 */
struct pollfd *grow_pollfds(struct pollfd *pollfds, int cur_count, int new_count);
#endif
