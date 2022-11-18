#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include <poll.h>

/* Esta função fecha o server com o 
 * sockfd dado nas situações de erro
 * e retorna -1
 */
int close_server_socket(int sockfd);

/* Esta função fecha a socket no indice index
 * do array dado
 */
void close_client_socket(struct pollfd *set, int index);

#endif