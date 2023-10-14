#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

int folder_exists(const char* filepath) {
    DIR* dir = opendir(filepath);
    if (dir != NULL) {
        closedir(dir);
        return 1; // 文件夹存在
    }
    return 0; // 文件夹不存在
}

int get_unique_client_number() {
    int counter=1;
    char folderpath[255];
    sprintf(folderpath,"./results/client%d_results",counter);
    while(folder_exists(folderpath)){
        counter++;
        sprintf(folderpath,"./results/client%d_results",counter);
    }
    return counter;
}

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

    int ucn=get_unique_client_number();
    char folder_path[255];
    sprintf(folder_path,"./results/client%d_results/",ucn);
    mkdir(folder_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    char string_ucn[255];
    sprintf(string_ucn, "%d", ucn);
    int msize = send(cd, string_ucn, 255, 0);

    printf("Enter a command for the server: ");

    char command[255];
    fgets(command, sizeof(command), stdin);
    msize = send(cd, command, 255, 0);

    char filename[255];
    msize = recv(cd, filename, sizeof(filename), 0);
    printf("Received the solution: %s\n", filename);
    
    char filepath[255];
    strcpy(filepath,folder_path);
    strcat(filepath,filename);
    FILE *file = fopen(filepath, "w");

    char buffer[1024];
    while(1){
        msize = recv(cd, buffer, sizeof(buffer), 0);
        if(msize == -1){
            perror("Receive file chunk failed");
        }
        else if(msize == 0){
            break;
        }else if(msize == 3 && memcmp(buffer, "EOF", 3) == 0){
            break;
        }

        // Write the received chunk to the file
        size_t bytesWritten = fwrite(buffer, 1, msize, file);
        if(bytesWritten < msize){
            perror("Write file chunk failed");
        }
    }
    fclose(file);
    
    return 0;
}
