/**
 * Grupo: SD-048
 * Autores:
 *   Francisco Correia - fc54685
 *   Alexandre Fonseca - fc55955
 *   Filipe Egipto - fc56272
 */
#include "network_server.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exit_bad_args()
{
    printf("Uso: ./tree-server <server>:<port>\n");
    printf("Exemplo de uso: ./tree-server 127.0.0.1:2181\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
    /* Testar os argumentos de entrada */
    if (argc != 2)
    {
        exit_bad_args();
    }

    char *sep = strchr(argv[1], ':');
    if (sep == NULL)
    {
        exit_bad_args();
    }

    char *address = strndup(argv[1], sep - argv[1]);
    u_int16_t port = atoi(sep + 1);
    if (port <= 0)
    {
        free(address);
        exit_bad_args();
    }

    int socket_de_escuta = network_server_init(htons(port));
    if (socket_de_escuta == -1)
    {
        free(address);
        exit(EXIT_FAILURE);
    }

    tree_skel_init(address);

    int result = network_main_loop(socket_de_escuta);
    result |= network_server_close();
    tree_skel_destroy();
    return result;
}
