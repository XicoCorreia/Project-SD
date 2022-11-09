/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "network_server.h"
#include "message-private.h"
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

void sigint_handler()
{
    printf("\nProcesso interrompido!\n");
    int result = network_server_close();
    tree_skel_destroy();
    exit(result);
}

int network_server_init(short port)
{
    signal_sigint(NULL); // ignora sinais SIGINT (e.g. CTRL)

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

    signal_sigint(sigint_handler);
    return sockfd;
}

int network_main_loop(int listening_socket)
{
    if (listen(listening_socket, 0) < 0)
    {
        perror("network_main_loop");
        return -1;
    }

    signal_sigpipe(NULL);

    struct sockaddr_in my_soc = {0};
    socklen_t addr_size = sizeof my_soc;
    int connsockfd = 0;

    while ((connsockfd = accept(listening_socket, (struct sockaddr *)&my_soc, &addr_size)) != -1)
    {
        printf("Ligação estabelecida com o cliente '%s:%d'\n", inet_ntoa(my_soc.sin_addr), my_soc.sin_port);
        MessageT *msg;
        while ((msg = network_receive(connsockfd)) != NULL)
        {
            int res;
            if ((res = invoke(msg)) != 0)
            {
                if (msg->c_type == MESSAGE_T__C_TYPE__CT_NONE)
                {
                    printf("Erro ao invocar a operação pedida pelo cliente.\n");
                }
            }
            network_send(connsockfd, msg);
        }

        if (close(connsockfd) < 0)
        {
            perror("network_main_loop");
        }
        printf("Foi fechada a ligação com o cliente.\n");
    }

    if (connsockfd < 0)
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

    nbytes = read_all(client_socket, &len, sizeof(int));
    if (nbytes != sizeof(int))
    {
        printf("Erro a receber dados do cliente.\n");
        close(client_socket);
        return NULL;
    }

    if (len == 0)
    {
        return NULL; // Ligação terminada pelo cliente
    }

    if (len < 0)
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

    nbytes = read_all(client_socket, buffer, len); // read_all
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
    return 0;
}
