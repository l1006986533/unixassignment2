# Define the compiler
CC = gcc

# Define the executable file 
CLIENT_TARGET = client/client
SERVER_TARGET = mathserver/server

# Define the C source files
CLIENT_SRC = client/client.c client/client_utils.c
SERVER_SRC = mathserver/src/server.c  mathserver/src/server_utils.c

SERVER_OBJ = mathserver/object/kmeans.o mathserver/object/matrix_inverse.o

INCLUDE_PATH = mathserver/include

all: $(CLIENT_TARGET) $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) -w -o $@ $^

$(SERVER_OBJ): mathserver/object/%.o : mathserver/include/%.c
	$(CC) -w -c -o $@ $<

$(SERVER_TARGET): $(SERVER_SRC) $(SERVER_OBJ)
	$(CC) -w -I$(INCLUDE_PATH) -O2 -o $@ $(SERVER_SRC) $(SERVER_OBJ)

clean:
	$(RM) $(CLIENT_TARGET) $(SERVER_TARGET) $(SERVER_OBJ)
	-rm -r client/results/* mathserver/computed_results/*
