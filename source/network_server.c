/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "network_server.h"
#include "message-private.h"
#include "network_server-private.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* Atribuir NULL permite realloc inicial (usando em grow_pollfds) e
 * evita free indeterminado devido a um SIGINT prematuro.
 */
struct pollfd *desc_set = NULL;
int nfdesc = 16;
int sockfd;

void sigint_handler()
{
    printf("\nProcesso interrompido!\n");
    int result = network_server_close();
    tree_skel_destroy();
    exit(result);
}

int network_server_init(short port)
{
    signal_sigint(NULL); // ignora sinais SIGINT (e.g. CTRL+C)

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

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
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

void close_client_socket(struct pollfd *set, int index)
{
    if (close(set[index].fd) == -1)
    {
        perror("close_client_socket");
    }
    set[index].fd = -1;
    printf("Foi fechada a ligação com o cliente.\n");
}

int network_main_loop(int listening_socket)
{
    signal_sigint(sigint_handler);

    struct sockaddr_in my_soc = {0};
    socklen_t addr_size = sizeof my_soc;

    if (listen(listening_socket, 0) < 0)
    {
        return -1;
    }

    int count = 1;
    desc_set = grow_pollfds(desc_set, count, nfdesc);
    if (desc_set == NULL)
    {
        return -1;
    }
    desc_set[0].fd = listening_socket;
    desc_set[0].events = POLLIN;

    signal_sigpipe(NULL);

    int i, k, error;
    while ((k = poll(desc_set, count, -1)) >= 0)
    {
        if (k > 0)
        {
            if ((desc_set[0].revents & POLLIN))
            {
                if (count == nfdesc) // crescer o array de descritores de sockets
                {
                    nfdesc *= 2;
                    desc_set = grow_pollfds(desc_set, count, nfdesc);
                    if (desc_set == NULL)
                    {
                        return -1;
                    }
                }
                if ((desc_set[count].fd = accept(desc_set[0].fd, (struct sockaddr *)&my_soc, &addr_size)) != -1)
                {
                    printf("Ligação estabelecida com o cliente '%s:%d'\n", inet_ntoa(my_soc.sin_addr), my_soc.sin_port);
                    desc_set[count].events = POLLIN;
                    count++;
                }
                else
                {
                    perror("network_main_loop");
                }
            }
            for (i = 1; i < count; i++)
            {
                if (desc_set[i].revents & POLLIN)
                {
                    MessageT *msg;
                    if (available_read_bytes(desc_set[i].fd) == 0)
                    {
                        close_client_socket(desc_set, i); // o cliente fechou a ligação (0 bytes para ler)
                        continue;
                    }

                    if ((msg = network_receive(desc_set[i].fd)) != NULL)
                    {
                        int res;
                        if ((res = invoke(msg)) != 0)
                        {
                            if (msg->c_type == MESSAGE_T__C_TYPE__CT_NONE)
                            {
                                printf("Erro ao invocar a operação pedida pelo cliente.\n");
                            }
                        }
                        if (network_send(desc_set[i].fd, msg) == -1)
                        {
                            close_client_socket(desc_set, i);
                        }
                    }
                    else // msg == NULL
                    {
                        close_client_socket(desc_set, i);
                    }
                }
                error = 0;
                socklen_t optlen = sizeof(int);
                getsockopt(desc_set[i].fd, SOL_SOCKET, SO_ERROR, &error, &optlen);
                if ((error != 0) || (desc_set[i].revents & POLLHUP))
                {
                    close_client_socket(desc_set, i);
                }
            }
        }
    }

    return 0;
}

MessageT *network_receive(int client_socket)
{
    int len, nbytes;
    void *buffer;

    nbytes = read_all(client_socket, &len, sizeof(int));
    if (nbytes != sizeof(int))
    {
        printf("Erro a receber dados do cliente.\n");
        return NULL;
    }

    if (len <= 0)
    {
        return NULL;
    }

    buffer = malloc(len);
    if (buffer == NULL)
    {
        perror("network_receive");
        return NULL;
    }

    nbytes = read_all(client_socket, buffer, len);
    if (nbytes != len)
    {
        printf("Erro a receber dados do cliente.");
        return NULL;
    }

    MessageT *msg = message_t__unpack(NULL, nbytes, buffer);
    free(buffer);
    return msg;
}

int network_send(int client_socket, MessageT *msg)
{
    int nbytes;
    int len = message_t__get_packed_size(msg);
    int offset = sizeof(int);
    void *buffer;

    buffer = malloc(len + offset);
    if (buffer == NULL)
    {
        perror("network_send");
        return -1;
    }

    // Serializar e enviar a mensagem
    memcpy(buffer, &len, offset);
    message_t__pack(msg, buffer + offset);
    message_t__free_unpacked(msg, NULL);
    if ((nbytes = write_all(client_socket, buffer, len + offset)) != len + offset)
    {
        printf("Erro a enviar dados para o cliente.");
        close(client_socket);
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
    free(desc_set);
    return 0;
}

struct pollfd *grow_pollfds(struct pollfd *pollfds, int cur_count, int new_count)
{
    if (new_count < 0)
    {
        printf("grow_pollfds: tamanho (%d) inválido.\n", new_count);
        return NULL;
    }
    else if (new_count < cur_count)
    {
        printf("grow_pollfds: tamanho (%d) inferior ao atual (%d).\n", new_count, cur_count);
        return NULL;
    }
    pollfds = reallocarray(pollfds, new_count, sizeof(struct pollfd));
    if (pollfds == NULL)
    {
        perror("grow_pollfds");
        return NULL;
    }
    for (; cur_count < new_count; cur_count++)
    {
        pollfds[cur_count].fd = -1;
        pollfds[cur_count].revents = 0;
    }
    return pollfds;
}
