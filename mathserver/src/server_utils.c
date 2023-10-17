#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

void handling_server_args(int argc, char** argv, int *port, char* originalWorkingDirectory){
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
                if (getcwd(originalWorkingDirectory, 1024) == NULL) {
                    perror("getcwd() error");
                    return 1;
                }
                daemonize("me");
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
            *k = atoi(strtok(NULL, " "));
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

void run_kmeans(int k, char* input_file, char* filepath){
    read_data(input_file);
    kmeans(k);
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


void daemonize(const char *cmd)
{
    int                 i, fd0, fd1, fd2;
    pid_t               pid;
    struct rlimit       rl;
    struct sigaction    sa;
    
    /* STEP 1: Clear file creation mask */
    umask(0);
    
    /* Get maximum number of file descriptors */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        perror(cmd);
        exit(1);
    }

    /* STEP 2a: Fork a child process */
    if ((pid = fork()) < 0) {
        perror(cmd);
        exit(1);
    }
    else if (pid != 0) { /* STEP 2b: Exit the parent process */
        exit(0);
    }
    /* STEP 3: Become a session leader to lose controlling TTY 
     * The child process executes this! */
    setsid(); 
    
    /* Ensure future opens won't allocate controlling TTYs */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        perror("Can't ignore SIGHUP");
        exit(1);
    }
    if ((pid = fork()) < 0){
        perror("Can't fork");
        exit(1);
    }
    else if (pid != 0) /* parent */
        exit(0);
    
    /*
     *      * Change the current working directory to the root so
     *           * we won't prevent file systems from being unmounted.
     *                */
    if (chdir("/") < 0){
        perror("Can't change to /");
        exit(1);
    }
    
    /* Close all open file descriptors */
    //printf("limit: %ld\n", rl.rlim_max);
    printf("The pid is: %d\n",getpid());
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);
    
    /* Attach file descriptors 0, 1, and 2 to /dev/null */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
}


int make_socket_non_blocking(int sfd) {
    int flags, s;
    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}