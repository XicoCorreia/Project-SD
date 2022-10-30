#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include <sys/types.h>

struct rtree_t
{
    u_int16_t port;
    char *address;
    int sockfd;
};

/* Função que liberta toda a memória alocada por rtree_get_keys().
 */
void rtree_free_keys(char **keys);

/* Função que liberta toda a memória alocada por rtree_get_values().
 */
void rtree_free_values(void **values);

#endif
