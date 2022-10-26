#include "network_server.h"

int main(int argc, char const *argv[])
{
    /* Testar os argumentos de entrada */
    if (argc != 2)
    {
        printf("Uso: ./tree-server <port>\n");
        printf("Exemplo de uso: ./tree-server 12345\n");
        return -1;
    }
    /* inicialização da camada de rede */
    int socket_de_escuta = network_server_init(htons(atoi(argv[1])));
    if (socket_de_escuta != -1)
    {
        tree_skel_init();
        int result = network_main_loop(socket_de_escuta);
        tree_skel_destroy();
        int close = network_server_close();
        if (close != 0)
        {
            return close;
        }
        return result;
    }
    return -1;
}
