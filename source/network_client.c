#include "network_client.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "client_stub-private.h"
int network_connect(struct rtree_t *rtree)
{
    int sockfd;
    struct sockaddr_in server;

    // * Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    // * Preenche estrutura server para estabelecer conexão
    server.sin_family = AF_INET;
    server.sin_port = htons(rtree->port);
    if (inet_pton(AF_INET, rtree->address, &server.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        close(sockfd);
        return -1;
    }

    // * Estabelece conexão com o servidor definido em server
    if (connect(sockfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(sockfd);
        return -1;
    }
    
    rtree->socketfd = sockfd;

    return 0;
}

MessageT *network_send_receive(struct rtree_t *rtree, MessageT *msg)
{
    return NULL;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t *rtree)
{   
    close(rtree->socketfd);
    return 0;
}
