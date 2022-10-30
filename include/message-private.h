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

/* Função que associa a invocação da função dada em argumento ao sinal
 * de "broken pipe". Se o argumento dado for NULL,
 * o sinal é ignorado através da constante SIG_IGN.
 */
void signal_sigpipe(void *handler);

/* Função que associa a invocação da função dada em argumento ao sinal
 * de interrupção do teclado (i.e., CTRL+C). Se o argumento dado for NULL,
 * o sinal é ignorado através da constante SIG_IGN.
 */
void signal_sigint(void *handler);

#endif
