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
    if (argc != 3)
    {
        printf("Uso: ./tree-server <port> <N>\n");
        printf("Exemplo de uso: ./tree-server 12345 3\n");
        return -1;
    }
    /* inicialização da camada de rede */
    int socket_de_escuta = network_server_init(htons(atoi(argv[1])));
    if (socket_de_escuta == -1)
    {
        return -1;
    }
    int num_threads = atoi(argv[2]);
    if (num_threads <= 0)
    {
        printf("Erro ao ler o número de threads especificado: %d\n", num_threads);
        return -1;
    }
    tree_skel_init(num_threads);
    int result = network_main_loop(socket_de_escuta);
    result |= network_server_close();
    tree_skel_destroy();
    return result;
}
