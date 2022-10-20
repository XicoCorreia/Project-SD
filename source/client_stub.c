#include "client_stub.h"
#include "client_stub-private.h"

struct rtree_t *rtree_connect(const char *address_port)
{
    return (struct rtree_t *)0;
}

int rtree_disconnect(struct rtree_t *rtree)
{
    return -1;
}

int rtree_put(struct rtree_t *rtree, struct entry_t *entry)
{
    return -1;
}

struct data_t *rtree_get(struct rtree_t *rtree, char *key)
{
    return (struct data_t *)0;
}

int rtree_del(struct rtree_t *rtree, char *key)
{
    return -1;
}

int rtree_size(struct rtree_t *rtree)
{
    return -1;
}

int rtree_height(struct rtree_t *rtree)
{
    return -1;
}

char **rtree_get_keys(struct rtree_t *rtree)
{
    return (char **)0;
}

void **rtree_get_values(struct rtree_t *rtree)
{
    return (void **)0;
}