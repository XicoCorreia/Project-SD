#ifndef _TREE_SKEL_PRIVATE_H
#define _TREE_SKEL_PRIVATE_H

#include "tree_skel.h"

#define OP_DEL 0
#define OP_PUT 1

/* Acrescenta um pedido (de escrita) à fila de pedidos.
 * O pedido será processado por uma thread secundária.
 */
void queue_add_request(request_t *request);

/* Obtém o primeiro pedido (de escrita) da fila de pedidos.
 * Se a fila estiver vazia, espera até que um pedido
 * esteja disponível para ser processado.
 */
request_t *queue_get_request();

/* Cria um pedido, reservando a memória necessária para o mesmo,
 * e copia os argumentos para a estrutura criada.
 */
request_t *create_request(int op_n, int op, char *key, struct data_t *data);

/* Esta função devolve o primeiro endereço IP que estiver associado
 * a uma das interfaces de allowed_ifs, ou null em caso contrário.
 */
char *get_if_addr(char **allowed_ifs, int n_addrs);

/* Esta função estabelece a ligação com o servidor ZooKeeper,
 * criando os nós relevantes no servidor para esse efeito.
 * O endereço de uma interface de rede válida (e.g. "eth0", "enp0s3")
 * é associado ao porto e a este servidor, e inserido no nó de ZooKeeper.
 */
int tree_skel_zookeeper_init(const char *zk_address_port, short port);

#endif
