#include "network_server.h"
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int MAX_MSG = 4096;
int sockfd;

int network_server_init(short port)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("network_server_init");
        return -1;
    }

    struct sockaddr_in my_soc;
    my_soc.sin_family = AF_INET;
    my_soc.sin_addr.s_addr = htonl(INADDR_ANY);
    my_soc.sin_port = port;

    int opt = 1;

    int sets = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (sets < 0)
    {
        perror("network_server_init");
        return -1;
    }

    if ((bind(sockfd, (struct sockaddr *)&my_soc, sizeof(my_soc))) < 0)
    {
        perror("network_server_init");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int network_main_loop(int listening_socket)
{
    if (listen(listening_socket, 0) < 0)
    {
        perror("network_main_loop");
        return -1;
    }

    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL); // chega ?

    struct sockaddr_in my_soc;
    socklen_t addr_size;
    int csock;

    while ((csock = accept(listening_socket, (struct sockaddr *)&my_soc, &addr_size)) != -1)
    {
        MessageT *msg = network_receive(csock);
        int res;
        if ((res = invoke(msg)) != 0)
        {
            return res; // erro
        }
        network_send(csock, msg);
        if (close(csock) < 0)
        {
            perror("network_main_loop");
            return -1;
        }
    }

    return 0;
}

MessageT *network_receive(int client_socket)
{
    void *buffer = malloc(MAX_MSG);
    int size = read(client_socket, buffer, MAX_MSG); // read_all
    if (size < 0)
    {
        perror("network_receive");
        return NULL;
    }
    return message_t__unpack(NULL, size, buffer);
}

int network_send(int client_socket, MessageT *msg)
{
    int len = message_t__get_packed_size(msg);
    void *buffer = malloc(len);
    if (buffer == NULL)
    {
        perror("network_send");
        return -1;
    }
    message_t__pack(msg, buffer);
    message_t__free_unpacked(msg, NULL);
    int size = write(client_socket, buffer, len); // write_all
    if (size != len)
    {
        perror("network_send");
        return -1;
    }
    free(buffer);
    return 0;
}

int network_server_close()
{
    if (close(sockfd) < 0)
    {
        perror("network_server_close");
        return -1;
    }
    return 0;
}