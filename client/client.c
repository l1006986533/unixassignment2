#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char const* argv[])
{

    int port=9999;
    char ip[255]="127.0.0.1";
    char* prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-'){
            if(strcmp(*argv,"-ip")==0){
                --argc;
                strcpy(ip, *++argv);
            }else if(strcmp(*argv,"-p")==0){
                --argc;
                port = atoi(*++argv);
            }else{
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -h \n\n", prog);
            }
        }
    printf("ip:%s,and port:%d\n",ip,port);
    int cd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &servAddr.sin_addr);
    int status = connect(cd, (struct sockaddr*)&servAddr,
                  sizeof(servAddr));
  
    if (status == -1) {
        printf("Error...\n");
    }
    else {
        printf("Connected to server\n");
    }

    printf("Enter a command for the server: ");

    char command[255];
    fgets(command, sizeof(command), stdin);
    int msize = send(cd, command, 255, 0);

    char filename[255];
    msize = recv(cd, filename, sizeof(filename), 0);
    printf("Received the solution: %s\n", filename);
    
    char filepath[255]="results/";
    strcat(filepath,filename);
    FILE *file = fopen(filepath, "w");

    char buffer[1024];
    while(1){
        msize = recv(cd, buffer, sizeof(buffer), 0);
        if(msize == -1){
            perror("Receive file chunk failed");
            // handle error
        }
        else if(msize == 0){
            // Connection closed
            break;
        }

        // Write the received chunk to the file
        size_t bytesWritten = fwrite(buffer, 1, msize, file);
        if(bytesWritten < msize){
            perror("Write file chunk failed");
            // handle error
        }
    }
    fclose(file);
    
    return 0;
}
