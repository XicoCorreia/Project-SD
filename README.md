# Sistemas Distribuídos 2022/2023

## Autores (Grupo SD-048)

- (fc54685) Francisco Correia
- (fc55955) Alexandre Fonseca
- (fc56272) Filipe Egipto

## Utilização

### Compilar e executar

No diretório de raíz do projeto, correr o seguinte:
```
$ make
$ ./binary/test_data
$ ./binary/test_entry
$ ./binary/test_tree
```
Por omissão, `make` compila os três ficheiros de teste fornecidos, assim como
`test_serialization` se este existir.

Para remover os ficheiros produzidos pelo compilador basta correr `make clean`.  
Também remove, quando aplicável, o ficheiro .zip produzido por `make zip`.

### Serialização

O formato de serialização de `char **keys` é o seguinte:

| Nº de byte |  0 (4 bytes)  |     4 (4 bytes)      | 8 (k bytes) |   8+k (4 bytes)   | ... |
| :--------: | :-----------: | :------------------: | :---------: | :---------------: | --- |
|   Valor    | nº de strings | k: tamanho 1ª string |  1ª string  | tamanho 2ª string | ... |
|    Tipo    |      int      |        char *        |     int     |      char *       | ... |

As strings **não** são *null-terminated*.