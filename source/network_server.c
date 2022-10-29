#include "network_server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int sockfd;

// Função que faz com que o sinal SIGPIPE seja ignorado.
void signal_sigpipe()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1)
    {
        perror("signal_sigpipe");
        exit(EXIT_FAILURE);
    }
}

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

int network_main_loop(int listening_socket)
{
    if (listen(listening_socket, 0) < 0)
    {
        perror("network_main_loop");
        return -1;
    }

    signal_sigpipe();

    struct sockaddr_in *my_soc = malloc(sizeof(struct sockaddr_in));
    socklen_t addr_size = 0;
    int connsockfd = 0;

    while ((connsockfd = accept(listening_socket, (struct sockaddr *)&my_soc, &addr_size)) != -1)
    {
        printf("Ligação estabelecida com o cliente '%s:%d'\n", inet_ntoa(my_soc->sin_addr), my_soc->sin_port);
        while (true)
        {
            MessageT *msg = network_receive(connsockfd);
            if (msg == NULL)
            {
                printf("Foi fechada a ligação com o cliente.\n");
                break;
            }
            int res;
            if ((res = invoke(msg)) != 0)
            {
                if (msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT)
                {
                    int err = *msg->data.data;
                    printf("Erro na mensagem recebida pelo servidor: %d\n", err);
                }
                return res; // erro
            }
            network_send(connsockfd, msg);
        }
    }

    free(my_soc);
    if (connsockfd < 0 || close(connsockfd) < 0)
    {
        perror("network_main_loop");
        return -1;
    }

    return 0;
}

MessageT *network_receive(int client_socket)
{
    int len, nbytes;
    void *buffer;

    nbytes = read(client_socket, &len, sizeof(int));
    if (nbytes != sizeof(int))
    {
        printf("Erro a receber dados do cliente.\n");
        close(client_socket);
        return NULL;
    }

    if (len <= 0)
    {
        printf("Tamanho de buffer pedido inválido: %d\n", len);
        close(client_socket);
        return NULL;
    }

    buffer = malloc(len);
    if (buffer == NULL)
    {
        perror("network_receive");
        return NULL;
    }

    nbytes = read(client_socket, buffer, len); // read_all
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
    if ((nbytes = write(client_socket, buffer, len + offset)) != len + offset)
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
    return 0;
}