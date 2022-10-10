#include "tree_skel.h"

int main(int argc, char const *argv[])
{
    /* Testar os argumentos de entrada */
    /* inicialização da camada de rede */
    int socket_de_escuta = network_server_init(/* */);
    tree_skel_init();
    int result = network_main_loop(socket_de_escuta);
    tree_skel_destroy();
    // TODO
    return 0;
}
