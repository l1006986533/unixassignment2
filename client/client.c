#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>

int port=9999;
char ip[255]="127.0.0.1";
int cd;

void handle_sigint(int sig) 
{ 
    send(cd, "client_closing", 15, 0);
    close(cd);
    printf("\n");
    exit(0);
} 

int main(int argc, char const* argv[])
{
    signal(SIGINT, handle_sigint);
    handling_client_args(argc, argv, &ip, &port);
    // printf("DEBUG: ip:%s,and port:%d\n",ip,port);
    
    cd = connect_to_server(ip, port);
    int ucn=get_unique_client_number();
    char folder_path[255];
    sprintf(folder_path,"./results/client%d_results/",ucn);
    mkdir(folder_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    char string_ucn[255];
    sprintf(string_ucn, "%d", ucn);
    send(cd, string_ucn, 255, 0);

    while(1){
        printf("Enter a command for the server: ");

        char command[255];
        fgets(command, sizeof(command), stdin);
        send(cd, command, 255, 0);

        if (strncmp(command, "kmeanspar", 9) == 0) {
            //get input file
            int k;
            char input_file[255]="kmeans-data.txt";
            handling_kmeans_args(command, &k, input_file);
            // send kmeans input file
            send_file(cd, input_file);
        }

        char filename[255];
        recv(cd, filename, 255, 0);
        printf("Received the solution: %s\n", filename);
        
        char filepath[255]; // filepath = folder_path + filename
        strcpy(filepath,folder_path);
        strcat(filepath,filename);
        recv_file(cd, filepath);
    }
    return 0;
}
