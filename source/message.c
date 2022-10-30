/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "message-private.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int read_all(int sock, void *buf, int len)
{
    int bufsize = len;
    while (len > 0)
    {
        int res = read(sock, buf, len);
        if (res < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("read_all");
            return res;
        }
        buf += res;
        len -= res;
    }
    return bufsize;
}

int write_all(int sock, void *buf, int len)
{
    int bufsize = len;
    while (len > 0)
    {
        int res = write(sock, buf, len);
        if (res < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("write_all");
            return res;
        }
        buf += res;
        len -= res;
    }
    return bufsize;
}
