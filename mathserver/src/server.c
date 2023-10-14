#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "kmeans.h"
#include "matrix_inverse.h"

void handle_client(int);

#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>

int is_socket_connected(int sock_fd) {
    char buffer[1];
    int err = recv(sock_fd, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);

    if (err == -1) {
        if (errno == ECONNRESET || errno == ENOTCONN || errno == ETIMEDOUT) {
            // socket is not connected
            return 0;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // no data available to read, socket may still be connected
            return 1;
        }
    } else if (err == 0) {
        // connection has been closed by the peer
        return 0;
    }

    // socket is connected
    return 1;
}


int main(int argc, char** argv)
{
    int port=9999;

    char* prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch (*++ * argv) {
            case 'h':
                printf("-h Print help text");
                printf("-p port Listen to port number port.");
                printf("-d Run as a daemon instead of as a normal program.");
                printf("-s strategy Specify the request handling strategy: fork, muxbasic, or muxscale");
                exit(0);
                break;
            case 'p':
                --argc;
                port = atoi(*++argv);
                break;
            case 'd':
                break;
            case 's':
                break;
            default:
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -h \n\n", prog);
                break;
            }

    // Create a socket
    int sd = socket(AF_INET, SOCK_STREAM, 0);
  
    // define server address
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr*)&servAddr, sizeof(servAddr))<0){  // Bind socket to the specified IP and port
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // listen for connections
    int listenres=listen(sd, 1);
    printf("Listening for clients...\n"); 


    while (1) {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int cd = accept(sd, &address, &addrlen); // integer to hold client socket.
        if(cd<0){
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        if(fork()==0){ //create child process
            close(sd);
            handle_client(cd);
            exit(0);
        }
        // parent process
        close(cd); //client file descriptor
    }
    return 0;
}


void handle_client(int cd){
    int flag=1;
    setsockopt(cd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));

    // receiving unique_client_number
    char string_ucn[255];
    int nbytes = recv(cd, string_ucn, sizeof(string_ucn), 0);
    int ucn=atoi(string_ucn);
    printf("Connected with client %d\n",ucn);

    while ( is_socket_connected(cd) )
    {
        // receiving command
        char command[255];
        nbytes = recv(cd, command, sizeof(command), 0);
        if(nbytes<=0) break;
        printf("Client %d commanded: %s", ucn, command);

        int sol_cnt=1;
        char filepath[255]="../computed_results/";
        char filename[255];
        if (strncmp(command, "matinvpar", 9) == 0) {
            
            int problemsize=5,p=1,max_num=15;
            char Initway[255]="fast";
            char *token = strtok(command, " ");
            while (token != NULL) {
                if(strcmp(token,"-n")==0){
                    problemsize = atoi(strtok(NULL, " "));
                }else if(strcmp(token,"-I")==0){
                    strcpy(Initway, strtok(NULL, " "));
                }else if(strcmp(token,"-P")==0){
                    p = atoi(strtok(NULL, " "));
                }else if(strcmp(token,"-m")==0){
                    max_num = atoi(strtok(NULL, " "));
                }
                token = strtok(NULL, " ");
            }

            sprintf(filename,"matinv_client%d_soln%d.txt",ucn,sol_cnt);
            strcat(filepath, filename);

            FILE* fp = fopen(filepath, "w");
            Init_Default(problemsize,Initway,max_num,p);
            Init_Matrix(fp);
            find_inverse();
            Save_Matrix_Result_As_File(fp);
        } else if (strncmp(command, "kmeanspar", 9) == 0) {

            int k;
            char filename_kmeans[255]="kmeans-data.txt";
            char *token = strtok(command, " ");
            while (token != NULL) {
                if(strcmp(token,"-k")==0){
                    k = atoi(strtok(NULL, " "));
                }else if(strcmp(token,"-f")==0){
                    strcpy(filename_kmeans, strtok(NULL, " "));
                }
                token = strtok(NULL, " ");
            }

            sprintf(filename,"kmeans_client%d_soln%d.txt",ucn,sol_cnt);
            strcat(filepath, filename);

            read_data(filename_kmeans); //filename
            kmeans(k);  //k
            write_results(filepath);
        } else{
            printf("Not start with matinvpar or kmeanspar\n");
            return;
        }

        printf("Sending solution: %s\n",filepath);

        FILE *file = fopen(filepath, "r");

        int msize = send(cd, filename, sizeof(filename), 0);
        if(msize < 0){
            perror("Send filename failed");
            // handle error
        }

        char buffer[255];

        while(!feof(file)){
            size_t bytesRead = fread(buffer, 1, 255, file);
            int bytesSent = send(cd, buffer, bytesRead, 0);
        }
        fclose(file);
        send(cd,"MyEOF",3,0);
    }
    printf("Client %d diconnect\n",ucn);
}
