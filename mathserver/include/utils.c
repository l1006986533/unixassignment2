#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void handling_server_args(int argc, char** argv, int *port){
    char* prog = *argv;
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
                *port = atoi(*++argv);
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
}

void handling_matinv_args(char *command, int *problemsize, int *p, int *max_num, char *Initway) {
    char *token = strtok(command, " ");
    while (token != NULL) {
        if(strcmp(token,"-n")==0){
            *problemsize = atoi(strtok(NULL, " "));
        }else if(strcmp(token,"-I")==0){
            strcpy(Initway, strtok(NULL, " "));
        }else if(strcmp(token,"-P")==0){
            *p = atoi(strtok(NULL, " "));
        }else if(strcmp(token,"-m")==0){
            *max_num = atoi(strtok(NULL, " "));
        }
        token = strtok(NULL, " ");
    }
}

void handling_kmeans_args(char *command, int *k, char *filename_kmeans){
    char *token = strtok(command, " ");
    while (token != NULL) {
        if(strcmp(token,"-k")==0){
            k = atoi(strtok(NULL, " "));
        }else if(strcmp(token,"-f")==0){
            strcpy(filename_kmeans, strtok(NULL, " "));
        }
        token = strtok(NULL, " ");
    }
}

void run_matinv(char* command, char* filepath){
    // set default value
    int problemsize = 5, p = 1, max_num = 15;
    char Initway[255] = "fast";
    // get value from command string
    handling_matinv_args(command, &problemsize, &p, &max_num, Initway);
    FILE* fp = fopen(filepath, "w");
    Init_Default(problemsize,Initway,max_num,p);
    Init_Matrix(fp);
    find_inverse();
    Save_Matrix_Result_As_File(fp);
}

void run_kmeans(char* command, char* filepath){
    int k;
    char filename_kmeans[255]="kmeans-data.txt";
    handling_kmeans_args(command, &k, filename_kmeans);
    read_data(filename_kmeans);
    kmeans(k);  //k
    write_results(filepath);
}

int bind_and_listen(int port){ //return socket file descriptor
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind failed");
        exit(1);
    }
    listen(sd, 1);
    printf("Listening for clients...\n"); 
    return sd;
}

void send_file(int cd, char* filepath, char* filename){
    //send filename
    send(cd, filename, 255, 0);

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
