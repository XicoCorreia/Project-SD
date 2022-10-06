#   Grupo: SD-048
# Autores:
#   Francisco Correia - fc54685
#   Alexandre Fonseca - fc55955
#   Filipe Egipto - fc56272
BIN_DIR = binary
INC_DIR = include
OBJ_DIR = object
SRC_DIR = source
OBJECTS = data.o entry.o serialization.o tree.o
TESTS = test_data.o test_entry.o test_tree.o test_serialization.o

data.o = data.h
entry.o = entry.h data.h
serialization.o = serialization.h data.h entry.h
tree.o = tree.h tree-private.h data.h

test_data.o = data.h
test_entry.o = data.h entry.h data.h
test_tree.o = data.h entry.h tree.h
test_serialization.o = serialization.h

# -std deve ser gnu99 ou superior para usar strdup
# em Ubuntu 22.04 (ambiente de labs), o gcc 11 usa -std=gnu++17
CC = gcc
CFLAGS = -std=gnu99 -g -Wall -I $(INC_DIR)
LIBS =

vpath %.o $(OBJ_DIR)

default: tests

tests: test_data test_entry test_tree test_serialization

test_data test_entry test_tree test_serialization: $(OBJECTS) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/, $(OBJECTS) $@.o) -o $(BIN_DIR)/$@ $(LIBS)

%.o: $(SRC_DIR)/%.c $($@)
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $<

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/*
