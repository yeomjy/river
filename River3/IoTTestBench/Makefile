CC=gcc
CCFLAGS=-Wall

BIN=bin
OBJ=obj
SRC=src

SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

CLIENT_SRCS := $(wildcard $(SRC)/process-wrapper*.c)
CLIENT_OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(CLIENT_SRCS))

SERVER_SRCS := $(wildcard $(SRC)/server*.c)
SERVER_OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SERVER_SRCS))

all: $(BIN)/process-wrapper $(BIN)/server

$(BIN)/process-wrapper: $(CLIENT_OBJECTS)
	test -d $(BIN) || mkdir -p $(BIN)
	$(CC) $(CCFLAGS) $(LFLAGS) $^ -o $@

$(BIN)/server: $(SERVER_OBJECTS)
	test -d $(BIN) || mkdir -p $(BIN)
	$(CC) $(CCFLAGS) $(LFLAGS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.c
	test -d $(OBJ) || mkdir -p $(OBJ)
	$(CC) $(CCFLAGS) $(LFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(BIN)

.PHONY: clean
