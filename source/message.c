/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "message-private.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
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

void signal_sigpipe(void *handler)
{
    struct sigaction sa;
    sa.sa_handler = handler == NULL ? SIG_IGN : handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1)
    {
        perror("signal_sigpipe");
        exit(EXIT_FAILURE);
    }
}

void signal_sigint(void *handler)
{
    struct sigaction sa;
    sa.sa_handler = handler == NULL ? SIG_IGN : handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("signal_sigint");
        exit(EXIT_FAILURE);
    }
}

int available_read_bytes(int fd)
{
    int nread = 0;
    ioctl(fd, FIONREAD, &nread);
    return nread;
}
