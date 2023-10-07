#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#include "kmeans.h"
#include "matrix_inverse.h"

void handle_client(int);

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
    printf("Client 1 commanded: %s", command);

    char filepath[255];
    char filename[255];
    if (strncmp(command, "matinvpar", 9) == 0) {
        FILE* fp = fopen("matinv_client1_soln1.txt", "w");
        Init_Default();		/* Init default values	*/
        // Read_Options(argc, argv);	/* Read arguments	*/
        Init_Matrix(fp);		/* Init the matrix	*/
        find_inverse();
        Save_Matrix_Result_As_File(fp);
        strcpy(filename,"matinv_client1_soln1.txt");
        strcpy(filepath,"./matinv_client1_soln1.txt");
    } else if (strncmp(command, "kmeanspar", 9) == 0) {
        read_data("kmeans-data.txt"); //filename
        kmeans(9);  //k
        write_results();
        strcpy(filename,"kmeans-data.txt");
        strcpy(filepath,"./kmeans-results.txt");
    } else{
        printf("Not start with matinvpar or kmeanspar");
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
    int msize = send(cd, solution, sizeof(solution), 0);
}