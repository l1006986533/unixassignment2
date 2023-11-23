# Define the compiler
CC = gcc

# Define the executable file 
CLIENT_TARGET = client/client
SERVER_TARGET = mathserver/server

# Define the C source files
CLIENT_SRC = client/client.c client/client_utils.c
SERVER_SRC = mathserver/src/server.c mathserver/include/kmeans.c mathserver/include/matrix_inverse.c mathserver/src/server_utils.c

INCLUDE_PATH = mathserver/include

all: $(CLIENT_TARGET) $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) -w -o $@ $^

$(SERVER_TARGET): $(SERVER_SRC)
	$(CC) -w -I$(INCLUDE_PATH) -O2 -o $@ $^

clean:
	$(RM) $(CLIENT_TARGET) $(SERVER_TARGET)
	-rm -r client/results/* mathserver/computed_results/*
