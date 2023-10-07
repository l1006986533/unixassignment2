#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>

int main(int argc, char const* argv[])
{
    int cd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(19009); // use some unused port number
    servAddr.sin_addr.s_addr = INADDR_ANY;
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

    char strData[102400];
    msize = recv(cd, strData, sizeof(strData), 0);
    
    char filepath[255]="results/";
    strcat(filepath,filename);
    FILE *file = fopen(filepath, "w");
    fputs(strData, file);
    fclose(file);
    
    return 0;
}
