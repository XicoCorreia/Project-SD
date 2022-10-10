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

Para remover os ficheiros produzidos pelo compilador basta correr `make clean`.  
Também remove, quando aplicável, o ficheiro .zip produzido por `make zip`.
