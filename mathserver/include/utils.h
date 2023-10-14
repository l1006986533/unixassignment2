void handling_server_args(int argc, char** argv, int *port);
void handling_kmeans_args(char *command, int *k, char *filename_kmeans);
void handling_matinv_args(char *command, int *problemsize, int *p, int *max_num, char *Initway);
void handle_matinv(char* command, char* filepath);
void handle_kmeans(char* command, char* filepath);