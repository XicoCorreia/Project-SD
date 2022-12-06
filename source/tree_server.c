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

int main(int argc, char const *argv[])
{
    /* Testar os argumentos de entrada */
    if (argc != 2)
    {
        printf("Uso: ./tree-server <IP>:<port>\n");
        printf("Exemplo de uso: ./tree-server 12345\n");
        return -1;
    }
    //sprintf
    /* inicialização da camada de rede */
    char *ipPort= malloc(sizeof(argv[1]));
    char *token = strtok(ipPort, ":");
    token = strtok(NULL, ":");
    
    int socket_de_escuta = network_server_init(htons(atoi(token)));
    if (socket_de_escuta == -1)
    {
        return -1;
    }

    tree_skel_init(argv[1]);

    int result = network_main_loop(socket_de_escuta);
    result |= network_server_close();
    tree_skel_destroy();
    return result;
}
