#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "kmeans.h"
#include "matrix_inverse.h"

void handle_client(int);

int file_exists(const char* filename) {
    char filepath[255]="../computed_results/";
    strcat(filepath,filename);
    FILE* file = fopen(filepath, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

void get_unique_filename(char* base_filename, int cd, char* result_filename) {
    int counter=1;
    sprintf(result_filename, "%s_client%d_soln%d.txt", base_filename, cd, counter);
    while (file_exists(result_filename)) {
        sprintf(result_filename, "%s_client%d_soln%d.txt", base_filename, cd, counter);
        counter++;
    }
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
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), ipstr, sizeof(ipstr));
        printf("Established connection with a client, socket fd is %d , ip is : %s , port : %d\n", 
                cd, ipstr, ntohs(address.sin_port));
        if(fork()==0){
            close(sd);
            handle_client(cd);
            exit(0);
        }
        // parent process
        close(cd);
    }
    return 0;
}


void handle_client(int cd){
    // receiving command
    char command[255];
    int nbytes = recv(cd, command, sizeof(command), 0);
    printf("Client %d commanded: %s", cd, command);
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

        get_unique_filename("matinv", cd, filename);
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

        get_unique_filename("kmeans", cd, filename);
        strcat(filepath, filename);
        read_data(filename_kmeans); //filename
        kmeans(k);  //k
        write_results(filepath);
    } else{
        printf("Not start with matinvpar or kmeanspar\n");
        return;
    }

    FILE *file = fopen(filepath, "r");

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char solution[102400];
    fread(solution, 1, fileSize, file);
    solution[fileSize] = '\0';
    fclose(file);

    printf("Sending solution: %s\n",filename);
    // First, send the filename
    int msize = send(cd, filename, strlen(filename) + 1, 0);
    // Then, send the content
    msize = send(cd, solution, sizeof(solution), 0);
}
