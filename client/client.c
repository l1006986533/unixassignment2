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

    char strToPrint[255];
    if(strncmp(command, "matinvpar", 9) == 0) 
        strcpy(strToPrint,"Received the solution: matinv_client1_soln1.txt\n");
    else if (strncmp(command, "kmeanspar", 9) == 0)
        strcpy(strToPrint,"Received the solution: kmeans_client1_soln1.txt\n");

    printf(strToPrint);
    char strData[102400];
    msize = recv(cd, strData, sizeof(strData), 0);
    
    FILE *file = fopen("results/client1_results.txt", "w");
    fputs(strData, file);
    fclose(file);
    
    return 0;
}
