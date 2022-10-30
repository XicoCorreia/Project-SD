# Sistemas Distribuídos 2022/2023 - Projeto (2a parte)

## Autores (Grupo SD-048)

- (fc54685) Francisco Correia
- (fc55955) Alexandre Fonseca
- (fc56272) Filipe Egipto

## Utilização

### Compilar e executar

No diretório de raíz do projeto, correr o seguinte:
```
$ make
$ tree-server <port>
$ tree-client <server>:<port>
```
Onde `server` é o endereço IP ou nome do servidor da árvore,
e `port` é o número do porto TCP onde o servidor está à espera de ligações.

Alvos do Makefile:

- `make [default]:` compila o cliente `tree-client` e o servidor `tree-server`
- `make proto`: produz os ficheiros `sdmessage.pb-c.c/.h` do ficheiro `.proto`
- `make clean`: limpa os ficheiros produzidos pelo compilador
- `make zip`: produz um ficheiro de arquivos para entrega do projeto

### Comandos do cliente

- `put <key> <data>`: cria ou substitui o valor `value` na entrada com a chave `key`
- `get <key>`: obtém o valor associado à chave `value` caso exista
- `del <key>`: apaga a entrada com a chave `key`
- `size`: devolve o tamanho da árvore
- `height`: devolve a altura da árvore
- `getkeys`: devolve uma lista ordenada de todas as chaves
- `getvalues`: devolve uma lista de todos os valores, ordenada pelas chaves respetivas
- `quit`: termina o programa
