#   Grupo: SD-048
# Autores:
#   Francisco Correia - fc54685
#   Alexandre Fonseca - fc55955
#   Filipe Egipto - fc56272
GROUP = grupo48
BIN_DIR = binary
INC_DIR = include
OBJ_DIR = object
SRC_DIR = source

CLIENT_OBJS = client-lib.o tree_client.o
CLIENT_LIB_OBJS = data.o client_stub.o entry.o network_client.o
SERVER_OBJS = data.o entry.o network_server.o tree_server.o tree_skel.o tree.o

# -std deve ser gnu99 ou superior para usar strdup
# em Ubuntu 22.04 (ambiente de labs), o gcc 11 usa -std=gnu++17
CC = gcc
CFLAGS = -std=gnu99 -Wall -g -I $(INC_DIR)
COMPILE = $(CC) $(CFLAGS) -c -o $(OBJ_DIR)/$@ $<

vpath %.o $(OBJ_DIR)

default: tree-client tree-server

tree-client: proto $(CLIENT_OBJS)
	$(CC) $(addprefix $(OBJ_DIR)/, $(CLIENT_OBJS)) -o $(BIN_DIR)/$@

tree-server: proto $(SERVER_OBJS)
	$(CC) $(addprefix $(OBJ_DIR)/, $(SERVER_OBJS)) -o $(BIN_DIR)/$@

client-lib.o: proto $(CLIENT_LIB_OBJS)
	ld -r $(addprefix $(OBJ_DIR)/, $(CLIENT_LIB_OBJS)) -o $(OBJ_DIR)/$@

%.o: $(SRC_DIR)/%.c $($@)
	$(COMPILE)

proto: sdmessage.proto
	protoc-c --c_out=. sdmessage.proto
	mv sdmessage.pb-c.c $(SRC_DIR)
	mv sdmessage.pb-c.h $(INC_DIR)

zip:
	make clean
	cd ../ && zip -r $(GROUP)-projeto2.zip $(GROUP) -x "*/.clang-format" "*/.git/*" \
	"*/.gitignore" "*/.vscode/*" \
	"*/.gitkeep"

clean:
	rm -f $(INC_DIR)/sdmessage.pb-c.h $(SRC_DIR)/sdmessage.pb-c.c \
	$(OBJ_DIR)/* $(BIN_DIR)/* ../$(GROUP)-projeto2.zip
