/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "network_server.h"
#include "tree_skel-private.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    int result = 0;
    /* Testar os argumentos de entrada */
    if (argc != 3)
    {
        printf("Uso: ./tree-server <port> <zk_server>:<zk_port>\n");
        printf("Exemplo de uso: ./tree-server 12345 127.0.0.1:2181\n");
        exit(EXIT_FAILURE);
    }
    /* inicialização da camada de rede */

    uint16_t port = atoi(argv[1]);
    int socket_de_escuta = network_server_init(htons(port));

    if (socket_de_escuta == -1)
    {
        return -1;
    }

    if (tree_skel_init(1) == -1) // uma thread secundária
    {
        return network_server_close();
    }

    if (tree_skel_zookeeper_init(argv[2], port) == 0)
    {
        result = network_main_loop(socket_de_escuta);
    }

    result |= network_server_close();
    tree_skel_destroy();
    return result;
}
