#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>

#define MAX_POINTS 4096
#define MAX_CLUSTERS 32

typedef struct point
{
    float x; // The x-coordinate of the point
    float y; // The y-coordinate of the point
    int cluster; // The cluster that the point belongs to
} point;

int	N_points;		// number of entries in the data
int k;      // number of centroids
point data[MAX_POINTS];		// Data coordinates
point cluster[MAX_CLUSTERS]; // The coordinates of each cluster center (also called centroid)

void read_data(char* filename, int k2)
{
    k = k2;
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Cannot open the file");
        exit(EXIT_FAILURE);
    }
   
    // Initialize points from the data file
    float temp;
    N_points = 0;
    while(fscanf(fp, "%f %f", &data[N_points].x, &data[N_points].y) != -1)
    {
        data[N_points].cluster = -1; // Initialize the cluster number to -1
        N_points++;
    }
    // printf("Read the problem data!\n");
    // Initialize centroids randomly
    srand(0); // Setting 0 as the random number generation seed
    for (int i = 0; i < k; i++)
    {
        int r = rand() % N_points;
        cluster[i].x = data[r].x;
        cluster[i].y = data[r].y;
    }
    fclose(fp);
}

int get_closest_centroid(int i, int k)  // time complexity: k
{
    /* find the nearest centroid */
    int nearest_cluster = -1;
    double xdist, ydist, dist, min_dist;
    min_dist = dist = INT_MAX;
    for (int c = 0; c < k; c++)
    { // For each centroid
        // Calculate the square of the Euclidean distance between that centroid and the point
        xdist = data[i].x - cluster[c].x;
        ydist = data[i].y - cluster[c].y;
        dist = xdist * xdist + ydist * ydist; // The square of Euclidean distance
        //printf("%.2lf \n", dist);
        if (dist <= min_dist)
        {
            min_dist = dist;
            nearest_cluster = c;
        }
    }
    //printf("-----------\n");
    return nearest_cluster;
}

typedef struct
{
    int begin;
    int end;
} rows_to_process;


void* to_parallel_func(void* arg){
    rows_to_process* tmp = (rows_to_process*)arg;
    int begin=tmp->begin;
    int end=tmp->end;
    bool something_changed_in_thread = false;
    for (int i = begin; i < end; i++)
    { // For each data point
        int old_cluster = data[i].cluster;
        int new_cluster = get_closest_centroid(i, k);
        data[i].cluster = new_cluster; // Assign a cluster to the point i
        if (old_cluster != new_cluster)
        {
            something_changed_in_thread = true;
        }
    }
    free(tmp);
    return (void*)something_changed_in_thread;
}

bool assign_clusters_to_points() // c: N*k
{
    bool something_changed = false;
    int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];
    for (int thread_num = 0; thread_num < NUM_THREADS; thread_num++)
    {
        int begin = N_points / NUM_THREADS * thread_num;
        int end = (thread_num == NUM_THREADS - 1) ? N_points : N_points / NUM_THREADS * (thread_num + 1);
        rows_to_process* tmp = malloc(sizeof(rows_to_process));
        tmp->begin=begin;  tmp->end=end;
        pthread_create(&threads[thread_num], NULL, to_parallel_func, tmp); //only one variable for the forth parameter of pthread_create
    }
    bool thread_result;
    for(int t=0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], (void**)&thread_result);
        something_changed = something_changed || thread_result;
    }
    return something_changed;
}

void update_cluster_centers() //c: N+k
{
    /* Update the cluster centers */
    int c;
    int count[MAX_CLUSTERS] = { 0 }; // Array to keep track of the number of points in each cluster
    point temp[MAX_CLUSTERS] = { 0.0 };

    for (int i = 0; i < N_points; i++)
    {
        c = data[i].cluster;
        count[c]++;
        temp[c].x += data[i].x;
        temp[c].y += data[i].y;
    }
    for (int i = 0; i < k; i++)
    {
        cluster[i].x = temp[i].x / count[i];
        cluster[i].y = temp[i].y / count[i];
    }
}

void kmeans()
{
    bool somechange;
    int iter = 0;
    do {
        iter++; // Keep track of number of iterations
        somechange = assign_clusters_to_points();
        update_cluster_centers();
    } while (somechange);
    // printf("Number of iterations taken = %d\n", iter);
    // printf("Computed cluster numbers successfully!\n");
}

void write_results(char* filename)
{
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Cannot open the file");
        exit(EXIT_FAILURE);
    }
    else
    {
        for (int i = 0; i < N_points; i++)
        {
            fprintf(fp, "%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
        }
    }
    // printf("Wrote the results to a file!\n");
    fclose(fp);
}
