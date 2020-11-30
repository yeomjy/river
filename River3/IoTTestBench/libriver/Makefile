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

LIB_SRCS := $(wildcard $(SRC)/libriver*.c)
LIB_OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(LIB_SRCS))

all: $(BIN)/process-wrapper $(BIN)/server $(BIN)/libriver.so

$(BIN)/process-wrapper: $(CLIENT_OBJECTS)
	test -d $(BIN) || mkdir -p $(BIN)
	@#$(CC) $(CCFLAGS) $(LFLAGS) $(CLIENT_OBJECTS) -lriver -L$(BIN) -Wl,-rpath=$(BIN) -o $@
	$(CC) $(CCFLAGS) $(LFLAGS) $(CLIENT_OBJECTS) -o $@

$(BIN)/server: $(SERVER_OBJECTS)
	test -d $(BIN) || mkdir -p $(BIN)
	$(CC) $(CCFLAGS) $(LFLAGS) $^ -o $@

$(BIN)/libriver.so: $(LIB_OBJECTS)
	test -d $(BIN) || mkdir -p $(BIN)
	$(CC) $(CCFLAGS) $(LFLAGS) -shared -Wl,-z,now $^ -o $@

$(OBJ)/libriver.o: $(LIB_SRCS)
	test -d $(OBJ) || mkdir -p $(OBJ)
	$(CC) $(CCFLAGS) $(LFLAGS) -fPIC -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c
	echo $<
	test -d $(OBJ) || mkdir -p $(OBJ)
	$(CC) $(CCFLAGS) $(LFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(BIN)

.PHONY: clean
