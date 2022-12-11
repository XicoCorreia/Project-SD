# Sistemas Distribuídos 2022/2023 - Projeto (4a parte)

## Autores (Grupo SD-048)

- (fc54685) Francisco Correia
- (fc55955) Alexandre Fonseca
- (fc56272) Filipe Egipto

## Utilização

### Compilar e executar

No diretório de raíz do projeto, correr o seguinte:

```shell
$ make
$ tree-server <port> <zk_server>:<zk_port>
$ tree-client <zk_server>:<zk_port>
```
Onde `zk_server` e `zk_port` são o endereço IP/nome e o porto TCP,
respetivamente, do servidor de ZooKeeper, e `port` é o porto TCP
onde o servidor escuta por ligações de clientes/servidores na cadeia.

Alvos do Makefile:

- `make [default]`: compila o cliente `tree-client` e o servidor `tree-server`
- `make proto`: produz os ficheiros `sdmessage.pb-c.c/.h` do ficheiro `.proto`
- `make clean`: limpa os ficheiros produzidos pelo compilador
- `make zip`: produz um ficheiro de arquivos para entrega do projeto
- `make withtree`: compila o cliente e o servidor, usando o tree_fornecido.o

Nota: Não foi incluído o ficheiro `tree.c` portanto deve ser colocado na pasta `./source`.
Como alternativa pode ser usado o alvo `make withtree`.

### Comandos do cliente

- `put <key> <data>`: (async) cria ou substitui o valor `value` na entrada com a chave `key`
- `get <key>`: obtém o valor associado à chave `value` caso exista
- `del <key>`: (async) apaga a entrada com a chave `key`
- `size`: devolve o tamanho da árvore
- `height`: devolve a altura da árvore
- `getkeys`: devolve uma lista ordenada de todas as chaves
- `getvalues`: devolve uma lista de todos os valores, ordenada pelas chaves respetivas
- `verify <op_n>`: devolve o estado de execução da operação `op_n`
- `quit` ou `exit`: termina o programa

### Notas: Ligação entre o cliente e o servidor

*Nesta parte do projeto, o cliente não verifica se foi encerrada a ligação
por parte do servidor através de 'poll'.*

Para terminar a execução de forma segura pode ser enviado o sinal SIGINT (e.g. CTRL+C).
Tipicamente os clientes encerram graciosamente quando detetam que o servidor encerrou.

O mesmo se aplica ao servidor quando um cliente termina a ligação, tanto através de SIGINT
como através do comando *quit/exit*.

### Notas: Limitações na gestão de novos clientes

A gestão no servidor de novas ligações é relativamente ingénua.
O vetor `desc_set` cresce tanto quanto necessário, duplicando de tamanho sucessivamente.

Dados os critérios de avaliação, mantivemos esta opção. Considerámos outras opções:

- manter uma pilha de posições recentemente libertadas e reaproveitá-las;
- descartar posições do vetor em que `fd == -1` antes de realocar memória (estilo GC);
- (incompatível com a assinatura da função `poll`) utilizar uma lista duplamente ligada.
