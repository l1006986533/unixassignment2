#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include "kmeans.h"
#include "matrix_inverse.h"

int port=9999;
int sd; //server file descriptor

void handle_client(int cd){
    // receiving unique_client_number
    char string_ucn[255];
    recv(cd, string_ucn, sizeof(string_ucn), 0);
    int ucn=atoi(string_ucn);
    printf("Connected with client %d\n",ucn);

    int matinv_sol_cnt = 1, kmeans_sol_cnt = 1;
    char command[255];
    while ( recv(cd, command, sizeof(command), 0) )
    {
        if(strncmp(command, "client_closing", 15) == 0) break;
        printf("Client %d commanded: %s", ucn, command);

        char filepath[255], filename[255];
        if (strncmp(command, "matinvpar", 9) == 0) {
            sprintf(filename,"matinv_client%d_soln%d.txt",ucn,matinv_sol_cnt++);
            sprintf(filepath,"../computed_results/%s",filename);
            run_matinv(command, filepath); // read the command, and write solution to filepath
        } else if (strncmp(command, "kmeanspar", 9) == 0) {
            sprintf(filename,"kmeans_client%d_soln%d.txt",ucn,kmeans_sol_cnt++);
            sprintf(filepath,"../computed_results/%s",filename);
            run_kmeans(command, filepath);
        } else{
            printf("Not start with matinvpar or kmeanspar\n");
            return;
        }
        printf("Sending solution: %s\n",filepath);
        send_file(cd, filepath, filename);
    }
    printf("Client %d diconnect\n",ucn);
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

int main(int argc, char** argv)
{
    handling_server_args(argc,argv,&port);
    sd = bind_and_listen(port);
    server_fork();
}
