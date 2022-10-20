#include "network_server.h"

int main(int argc, char const *argv[])
{
    /* Testar os argumentos de entrada */
    /* inicialização da camada de rede */
    int socket_de_escuta = network_server_init(45683);
    tree_skel_init();
    int result = network_main_loop(socket_de_escuta);
    tree_skel_destroy();
    // TODO
    return 0;
}
