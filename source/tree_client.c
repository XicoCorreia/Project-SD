#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <client_stub.h>
#include <client_stub-private.h>


int main(int argc, char const *argv[])
{
    struct rtree_t *rtree = rtree_connect(argv[1]);

    while(true){
        char str[50];
        fgets(str,sizeof(str),stdin);
        str[strlen(str) - 1] = '\0';

        char* token = strtok(str, " ");
        
        if(strcmp(token, "put") == 0){      

            token = strtok(NULL, " ");
            char* key = malloc(sizeof(char*) * sizeof(token));
            char* value = malloc(sizeof(char*) * 100);             // ! Tem tamanho definido

            token = strtok(NULL, " ");
            while(token != NULL) {
                strcat(value, token);
                token = strtok(NULL, " ");
                if(token != NULL)
                    strcat(value, " ");
            }

            struct data_t* data = data_create2(strlen(value), value);
            struct entry_t* entry = entry_create(key, data);

            int i = rtree_put(rtree, entry); 
            if(i == -1){
                perror("Erro no comando put\n");    
                return -1;
            }
        }
        else if(strcmp(token, "get") == 0){     
            char* key = strtok(NULL, " ");
            struct data_t* data = rtree_get(rtree, key);
            if(data == NULL){
                perror("Erro no comando get\n");    
                return -1;
            }
        }
        else if(strcmp(token, "del") == 0){    
            char* key = strtok(NULL, " ");
            int i = rtree_del(rtree, key);  
            if(i == -1){
                perror("Erro no comando del\n");    
                return -1;
            }
        }
        else if(strcmp(token, "size") == 0){
            int size = rtree_size(rtree);
            if(size == -1){
                perror("Erro no comando size\n");    
                return -1;
            }
            else
                printf("%d\n", size);
        }
        else if(strcmp(token, "height") == 0){
            int height = rtree_height(rtree);
            if(height == -1){
                perror("Erro no comando height\n");    
                return -1;
            } 
            else
                printf("%d\n", height);
        }
        else if(strcmp(token, "getkeys") == 0){
            char** keys = rtree_get_keys(rtree);
            printf("Correu\n");
        }
        else if(strcmp(token, "getvalues") == 0){
            void** values = rtree_get_values(rtree);
            printf("Correu\n");
            
        }
        else if(strcmp(token, "quit") == 0){   
            int i = rtree_disconnect(rtree);            // ? Colocar o disconnect fora do while
            if(i == -1){
                perror("Erro no comando quit\n");    
                return -1;
            }
            break;
        }
    }
    return 0;
}