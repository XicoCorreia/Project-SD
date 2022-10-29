#include "network_client.h"
#include "client_stub-private.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

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

int network_connect(struct rtree_t *rtree)
{
    int sockfd;
    struct sockaddr_in server;

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    // Preenche estrutura server para estabelecer conexão
    server.sin_family = AF_INET;
    server.sin_port = htons(rtree->port);
    if (inet_pton(AF_INET, rtree->address, &server.sin_addr) < 1)
    {
        perror("Erro ao converter IP");
        close(sockfd);
        return -1;
    }

    // Estabelece conexão com o servidor definido em server
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Erro ao conectar-se ao servidor");
        close(sockfd);
        return -1;
    }

    rtree->sockfd = sockfd;

    signal_sigpipe();

    return 0;
}

MessageT *network_send_receive(struct rtree_t *rtree, MessageT *msg)
{
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
    if ((nbytes = write(sockfd, buffer, len + offset)) != len + offset)
    {
        perror("network_send_receive - write");
        close(sockfd);
        return NULL;
    }

    // Ler tamanho de buffer a alocar
    if ((nbytes = read(sockfd, &len, offset)) != offset)
    {
        perror("network_send_receive - read");
        close(sockfd);
        return NULL;
    }

    if (len <= 0)
    {
        perror("network_send_receive - invalid buffer size received");
        close(sockfd);
        return NULL;
    }

    if ((buffer = realloc(buffer, len)) == NULL)
    {
        perror("network_send_receive - realloc");
        close(sockfd);
        return NULL;
    }

    if ((nbytes = read(sockfd, buffer, len)) != len)
    {
        perror("network_send_receive");
        close(sockfd);
        return NULL;
    }
    // De-serializar mensagem recebida
    msg = message_t__unpack(NULL, nbytes, buffer); // ? Argumentos certos
    free(buffer);
    return msg;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t *rtree)
{
    if (close(rtree->sockfd) < 0)
    {
        perror("network_close");
        return -1;
    }
    return 0;
}
