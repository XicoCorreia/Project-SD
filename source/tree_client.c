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
        char* token;                            //? Alocar mem
        token = strtok(str, " ");

        if(strcmp(token, "put") == 0){
            char* key = strtok(NULL, " ");
            //TODO: DATA AND GET
        }
        else if(strcmp(token, "get") == 0){     //? CASO FALHE
            char* key = strtok(NULL, " ");
            struct data_t* data = rtree_get(rtree, key);
            if(data == NULL)
                printf("FALHOU GET");
        }
        else if(strcmp(token, "del") == 0){     //? CASO FALHE
            char* key = strtok(NULL, " ");
            int i = rtree_del(rtree, key);  
            if(i == -1)
                printf("FALHOU DEL");
        }
        else if(strcmp(token, "size") == 0){
            int size = rtree_size(rtree);
        }
        else if(strcmp(token, "height") == 0){
            int height = rtree_height(rtree);
        }
        else if(strcmp(token, "getkeys") == 0){
            char** keys = rtree_get_keys(rtree);
        }
        else if(strcmp(token, "getvalues") == 0){
            void** values = rtree_get_values(rtree);
        }
        else if(strcmp(token, "quit") == 0){    //? CASO FALHE
            int i = rtree_disconnect(rtree);        
            if(i == -1)
                printf("FALHOU QUIT");
            break;
        }
    }
    return 0;
}