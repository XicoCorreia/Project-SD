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
LIB_DIR = lib

CLIENT_OBJS = tree_client.o
CLIENT_LIB_OBJS = data.o client_stub.o entry.o message.o network_client.o sdmessage.pb-c.o
SERVER_OBJS = network_server.o tree_server.o tree_skel.o tree.o

# -std deve ser gnu99 ou superior para usar strdup
# em Ubuntu 22.04 (ambiente de labs), o gcc 11 usa -std=gnu++17
CC = gcc
CFLAGS = -std=gnu99 -Wall -g -I $(INC_DIR)
COMPILE = $(CC) $(CFLAGS) -c -o $(OBJ_DIR)/$@ $<
LIBS = -l:libprotobuf-c.a -lpthread -lzookeeper_mt

vpath %.o $(OBJ_DIR)

default: tree-client tree-server

withtree:
	cp tree_fornecido.o $(OBJ_DIR)/tree.o
	make default

tree-client: client-lib.o $(CLIENT_OBJS)
	$(CC) $(addprefix $(OBJ_DIR)/, $(CLIENT_OBJS)) $(LIB_DIR)/client-lib.o -o $(BIN_DIR)/$@ $(LIBS)

tree-server: client-lib.o $(SERVER_OBJS)
	$(CC) $(addprefix $(OBJ_DIR)/, $(SERVER_OBJS)) $(LIB_DIR)/client-lib.o -o $(BIN_DIR)/$@ $(LIBS)

client-lib.o: $(CLIENT_LIB_OBJS)
	ld -r $(addprefix $(OBJ_DIR)/, $(CLIENT_LIB_OBJS)) -o $(LIB_DIR)/$@

%.o: $(SRC_DIR)/%.c $($@)
	$(COMPILE)

%.pb-c.c: %.proto
	protoc-c $*.proto --c_out=./
	mv $*.pb-c.c $(SRC_DIR)
	mv $*.pb-c.h $(INC_DIR)

proto: sdmessage.pb-c.c

zip:
	make clean
	cd ../ && zip -r $(GROUP)-projeto4.zip $(GROUP) -x "*/.clang-format" "*/.git/*" \
	"*/.gitignore" "*/.vscode/*" "*/.gitkeep" "*/*.pdf"

clean:
	rm -f $(OBJ_DIR)/* $(LIB_DIR)/* $(BIN_DIR)/* ../$(GROUP)-projeto4.zip
