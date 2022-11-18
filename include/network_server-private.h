#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include <poll.h>

/* Esta função fecha o socket com o descritor de ficheiro dado.
 * sockfd deve corresponder ao socket do servidor que aceita novos clientes.
 * Devolve o código de erro (errno) e imprime o erro, caso exista, em stderr.
 */
int close_server_socket(int sockfd);

/* Esta função fecha o socket no índice do array de pollfd dado.
 * O valor do descritor na estrutura é colocado a -1 após fechar o socket.
 */
void close_client_socket(struct pollfd *set, int index);

#endif
