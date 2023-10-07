#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
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

int main(int argc, char const* argv[])
{
    // Create a socket
    int sd = socket(AF_INET, SOCK_STREAM, 0);
  
    // define server address
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(19009);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr*)&servAddr, sizeof(servAddr))<0){  // Bind socket to the specified IP and port
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // listen for connections
    int listenres=listen(sd, 1);
    printf("Accepting connections...\n"); 


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
        get_unique_filename("matinv", cd, filename);
        strcat(filepath, filename);
        FILE* fp = fopen(filepath, "w");
        Init_Default();		/* Init default values	*/
        // Read_Options(argc, argv);	/* Read arguments	*/
        Init_Matrix(fp);		/* Init the matrix	*/
        find_inverse();
        Save_Matrix_Result_As_File(fp);
    } else if (strncmp(command, "kmeanspar", 9) == 0) {
        get_unique_filename("kmeans", cd, filename);
        strcat(filepath, filename);
        read_data("kmeans-data.txt"); //filename
        kmeans(9);  //k
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