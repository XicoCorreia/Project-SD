/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "network_client.h"
#include "client_stub-private.h"
#include "message-private.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int network_connect(struct rtree_t *rtree)
{
    int sockfd;
    struct sockaddr_in server;

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("network_connect");
        return -1;
    }

    struct addrinfo hints = {};
    struct addrinfo *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(rtree->address, NULL, &hints, &res) != 0)
    {
        perror("network_connect");
        close(sockfd);
        return -1;
    }

    server = *(struct sockaddr_in *)res->ai_addr;

    // Preenche estrutura server para estabelecer conexão
    server.sin_family = AF_INET;
    server.sin_port = htons(rtree->port);

    // Estabelece conexão com o servidor definido em server
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("network_connect");
        freeaddrinfo(res);
        close(sockfd);
        return -1;
    }

    signal_sigpipe(NULL);
    freeaddrinfo(res);
    rtree->sockfd = sockfd;
    return 0;
}

MessageT *network_send_receive(struct rtree_t *rtree, MessageT *msg)
{
    MessageT *result;
    int sockfd = rtree->sockfd;
    int nbytes;
    int len = message_t__get_packed_size(msg);
    int offset = sizeof(int);
    void *buffer;

    buffer = malloc(len + offset);
    if (buffer == NULL)
    {
        perror("network_send");
        return NULL;
    }

    // Serializar e enviar a mensagem
    memcpy(buffer, &len, offset);
    message_t__pack(msg, buffer + offset);
    if ((nbytes = write_all(sockfd, buffer, len + offset)) != len + offset)
    {
        printf("Erro a enviar dados para o servidor.");
        close(sockfd);
        return NULL;
    }

    // Ler tamanho de buffer a alocar
    if ((nbytes = read_all(sockfd, &len, offset)) != offset)
    {
        printf("Erro a receber dados do servidor.");
        close(sockfd);
        return NULL;
    }

    if (len <= 0)
    {
        printf("Tamanho de buffer pedido inválido: %d\n", len);
        close(sockfd);
        return NULL;
    }

    if ((buffer = realloc(buffer, len)) == NULL)
    {
        perror("network_send_receive");
        close(sockfd);
        return NULL;
    }

    if ((nbytes = read_all(sockfd, buffer, len)) != len)
    {
        printf("Erro a receber dados do servidor.");
        close(sockfd);
        return NULL;
    }

    result = message_t__unpack(NULL, nbytes, buffer);
    *msg = *result;
    free(result);
    free(buffer);
    return msg;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t *rtree)
{
    // Sinalizamos ao servidor que pretendemos encerrar a ligação
    int len = 0;
    if (write_all(rtree->sockfd, &len, sizeof(int)) < 0)
    {
        perror("network-close");
    }
    if (close(rtree->sockfd) < 0)
    {
        perror("network_close");
        return -1;
    }
    return 0;
}
