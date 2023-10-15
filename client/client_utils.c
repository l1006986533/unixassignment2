#include <stdio.h>
#include <netinet/in.h>
#include <dirent.h>
#include <string.h>

int connect_to_server(char* ip, int port){
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
    return cd;
}

int folder_exist(const char* filepath) {
    DIR* dir = opendir(filepath);
    if (dir != NULL) {
        closedir(dir);
        return 1; // folder exist
    }
    return 0; // folder not exist
}

int get_unique_client_number() {
    int counter=1;
    char folderpath[255];
    sprintf(folderpath,"./results/client%d_results",counter);
    while(folder_exist(folderpath)){
        counter++;
        sprintf(folderpath,"./results/client%d_results",counter);
    }
    return counter;
}

void recv_file(int cd, char* filepath){
    FILE *file = fopen(filepath, "w");
    char buffer[256];
    buffer[255]='\0';
    while(1){
        int msize = recv(cd, buffer, 255, 0);
        if(msize == 0 || (msize == 5 && memcmp(buffer, "MyEOF", 5) == 0 )) break;
        fwrite(buffer, 1, strlen(buffer), file);
    }
    fclose(file);
}

void send_file(int cd, char* filepath){
    FILE *file = fopen(filepath, "r");
    char buffer[255];
    while(!feof(file)){
        size_t bytesRead = fread(buffer, 1, 255, file);
        if(bytesRead<255) buffer[bytesRead]='\0';
        send(cd, buffer, 255, 0);
    }
    fclose(file);
    send(cd,"MyEOF",5,0);
}

void handling_kmeans_args(char *command, int *k, char *filename_kmeans){
    char *token = strtok(command, " ");
    while (token != NULL) {
        if(strcmp(token,"-k")==0){
            *k = atoi(strtok(NULL, " "));
        }else if(strcmp(token,"-f")==0){
            strcpy(filename_kmeans, strtok(NULL, " "));
        }
        token = strtok(NULL, " ");
    }
}

void handling_client_args(int argc, char const* argv[], char* ip, int* port){
    char* prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-'){
            if(strcmp(*argv,"-ip")==0){
                --argc;
                strcpy(ip, *++argv);
            }else if(strcmp(*argv,"-p")==0){
                --argc;
                *port = atoi(*++argv);
            }else{
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -h \n\n", prog);
            }
        }
}