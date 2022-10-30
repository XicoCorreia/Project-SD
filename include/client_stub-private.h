#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"
#include <sys/types.h>

struct rtree_t
{
    // TODO
    u_int16_t port;
    char *address;
    int sockfd;
};

#endif
