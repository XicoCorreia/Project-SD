#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

/**
 * Esta função lê o número de bytes desejado do socket sockfd,
 * guardando-os no buffer dado.
 * Esta função tolera interrupções do sistema definidas pelo erro EINTR.
 */
int read_all(int sockfd, void *buf, int len);

/**
 * Esta função escreve o número de bytes desejado no socket sockfd,
 * lendo-os do buffer dado.
 * Esta função tolera interrupções do sistema definidas pelo erro EINTR.
 */
int write_all(int sockfd, void *buf, int len);

#endif