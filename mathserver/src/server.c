#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include "kmeans.h"
#include "matrix_inverse.h"
#include "utils.h"

void handle_client(int);

int port=9999;
int sd; //server file descriptor

void bind_and_listen(){
    sd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    listen(sd, 1);
    printf("Listening for clients...\n"); 
}

void server_fork(){
    while (1) {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int cd = accept(sd, &address, &addrlen); // client file descriptor
        if (cd < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        if (fork() == 0) { // create child process
            close(sd);
            handle_client(cd);
            exit(0);
        }
        // parent process
        close(cd);
    }
}

int is_socket_connected(int sock_fd) {
    char buffer[1];
    int msize = recv(sock_fd, buffer, sizeof(buffer), MSG_PEEK);
    if (msize == -1) {
        return 0;
    }
    return 1;
}

void send_file(int cd, char* filepath, char* filename){
    //send filename
    int msize = send(cd, filename, 255, 0);
    if(msize < 0){
        perror("Send filename failed");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(filepath, "r");
    char buffer[255];
    while(!feof(file)){
        size_t bytesRead = fread(buffer, 1, 255, file);
        send(cd, buffer, 255, 0);
    }
    fclose(file);
    send(cd,"MyEOF",5,0);
}

int main(int argc, char** argv)
{
    handling_server_args(argc,argv,&port);
    bind_and_listen();
    server_fork();
}


void handle_client(int cd){
    // receiving unique_client_number
    char string_ucn[255];
    recv(cd, string_ucn, sizeof(string_ucn), 0);
    int ucn=atoi(string_ucn);
    printf("Connected with client %d\n",ucn);

    int matinv_sol_cnt = 1, kmeans_sol_cnt = 1;

    while ( is_socket_connected(cd) )
    {
        // receiving command
        char command[255];
        recv(cd, command, sizeof(command), 0);
        printf("Client %d commanded: %s", ucn, command);

        char filepath[255], filename[255];
        if (strncmp(command, "matinvpar", 9) == 0) {
            sprintf(filename,"matinv_client%d_soln%d.txt",ucn,matinv_sol_cnt++);
            sprintf(filepath,"../computed_results/%s",filename);
            handle_matinv(command, filepath); // read the command, and write result to filepath
        } else if (strncmp(command, "kmeanspar", 9) == 0) {
            sprintf(filename,"kmeans_client%d_soln%d.txt",ucn,kmeans_sol_cnt++);
            sprintf(filepath,"../computed_results/%s",filename);
            handle_kmeans(command, filepath);
        } else{
            printf("Not start with matinvpar or kmeanspar\n");
            return;
        }
        printf("Sending solution: %s\n",filepath);
        send_file(cd, filepath, filename);
    }
    printf("Client %d diconnect\n",ucn);
}
