#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include "kmeans.h"
#include "matrix_inverse.h"

int port=9999;
int sd; //server file descriptor
char originalWorkingDirectory[1024];

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
            sprintf(filepath,"%s/../computed_results/%s",originalWorkingDirectory,filename);
            run_matinv(command, filepath); // read the command, and write solution to filepath
        } else if (strncmp(command, "kmeanspar", 9) == 0) {
            sprintf(filename,"kmeans_client%d_soln%d.txt",ucn,kmeans_sol_cnt++);
            sprintf(filepath,"%s/../computed_results/%s",originalWorkingDirectory,filename);
            int k;
            char input_file[255]="kmeans-data.txt";
            handling_kmeans_args(command, &k, input_file);

            //receive input file
            char temp[255];
            sprintf(temp,"%s/../computed_results/client%d-%s",originalWorkingDirectory,ucn,input_file);
            strcpy(input_file, temp);
            recv_file(cd, input_file);

            run_kmeans(k, input_file, filepath); //get the parameters k and input_file, and write solution to filepath
        } else{
            printf("Not start with matinvpar or kmeanspar\n");
            continue;
        }
        printf("Sending solution: %s\n",filename);
        send(cd, filename, 255, 0);
        send_file(cd, filepath);
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
    handling_server_args(argc,argv,&port,originalWorkingDirectory);
    sd = bind_and_listen(port);
    server_fork();
}
