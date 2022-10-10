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
OBJECTS = data.o entry.o serialization.o tree.o

data.o = data.h
entry.o = entry.h data.h
tree.o = tree.h tree-private.h data.h

# -std deve ser gnu99 ou superior para usar strdup
# em Ubuntu 22.04 (ambiente de labs), o gcc 11 usa -std=gnu++17
CC = gcc
CFLAGS = -std=gnu99 -g -Wall -I $(INC_DIR)
LIBS =

vpath %.o $(OBJ_DIR)

default: tree-client tree-server

tree-client: $(OBJECTS) tree-client.o
	$(CC) $(addprefix $(OBJ_DIR)/, $(OBJECTS) $@.o) -o $(BIN_DIR)/$@ $(LIBS)

tree-server: $(OBJECTS) tree-server.o
	$(CC) $(addprefix $(OBJ_DIR)/, $(OBJECTS) $@.o) -o $(BIN_DIR)/$@ $(LIBS)

# client.o:
# Usar linker ld com opção -r: ld -r in1.o in2.o [...] -o client-lib.o

%.o: $(SRC_DIR)/%.c $($@)
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $<

zip:
	make clean
	cd ../ && zip -r $(GROUP)-projeto2.zip $(GROUP) -x "*/.clang-format" "*/.git/*" \
	"*/.gitignore" "*/.vscode/*" \
	"*/.gitkeep"

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/* ../$(GROUP)-projeto2.zip
