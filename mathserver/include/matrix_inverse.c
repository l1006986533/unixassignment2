/***************************************************************************
 *
 * Sequential version of Matrix Inverse
 * An adapted version of the code by H�kan Grahn
 ***************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "matrix_inverse.h"

int	N;		/* matrix size		*/
int	maxnum;		/* max number of element*/
char* Init;		/* matrix init type	*/
int	PRINT;		/* print switch		*/
matrix A;		/* matrix A		*/
matrix I = {0.0};  /* The A inverse matrix, which will be initialized to the identity matrix */

typedef struct
{
    int begin; 
    int end;
    int p;   // p means column to eliminate
} elimination;


void* parallel_func(void* arg){
    elimination* tmp=arg;
    int p=tmp->p;
    for (int row = tmp->begin; row < tmp->end; row++) {   //pick row 
        double multiplier = A[row][p];
        if (row != p) // Perform elimination on all except the current pivot row 
        {
            for (int col = 0; col < N; col++) //operating every number in this row  /  every column in one row   parallel
            {
                A[row][col] = A[row][col] - A[p][col] * multiplier; /* Elimination step on A */
                I[row][col] = I[row][col] - I[p][col] * multiplier; /* Elimination step on I */
            }      
            assert(A[row][p] == 0.0);
        }
    }
    free(tmp);
}

void find_inverse() // time complexity: N*N*N
{
    int row, p; // 'p' stands for pivot (numbered from 0 to N-1)
    double pivalue; // pivot value

    /* Bringing the matrix A to the identity form */
    for (p = 0; p < N; p++) { /* Outer loop */     //pick col
        pivalue = A[p][p];
        for (int col = 0; col < N; col++)
        {
            A[p][col] = A[p][col] / pivalue; /* Division step on A */
            I[p][col] = I[p][col] / pivalue; /* Division step on I */
        }
        assert(A[p][p] == 1.0);

        int NUM_THREADS = 4;
        pthread_t threads[NUM_THREADS];

        for (int thread_num = 0; thread_num < NUM_THREADS; thread_num++)
        {
            int begin = N / NUM_THREADS * thread_num;
            int end = (thread_num == NUM_THREADS - 1) ? N : N / NUM_THREADS * (thread_num + 1);
            elimination* pack = malloc(sizeof(elimination));
            pack->begin=begin; pack->end=end; pack->p=p;
            pthread_create(&threads[thread_num], NULL, parallel_func, pack);
        }

        for(int t = 0; t < NUM_THREADS; t++) {
            pthread_join(threads[t], NULL);
        }
    }
}

void Init_Matrix(FILE* fp)
{
    int row, col;

    // Set the diagonal elements of the inverse matrix to 1.0
    // So that you get an identity matrix to begin with
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            if (row == col)
                I[row][col] = 1.0;
        }
    }
    fprintf(fp,"Matrix Inverse\n");
    fprintf(fp, "\nsize      = %dx%d ", N, N);
    fprintf(fp, "\nmaxnum    = %d \n", maxnum);
    fprintf(fp, "Init	  = %s \n", Init);
    fprintf(fp, "Initializing matrix...");

    if (strcmp(Init, "rand") == 0) {
        srand(0);
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) /* diagonal dominance */
                    A[row][col] = (double)(rand() % maxnum) + 5.0;
                else
                    A[row][col] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init, "fast") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) /* diagonal dominance */
                    A[row][col] = 5.0;
                else
                    A[row][col] = 2.0;
            }
        }
    }

    fprintf(fp, "done \n\n");
    if (PRINT == 1)
    {
        //Print_Matrix(A, "Begin: Input");
        //Print_Matrix(I, "Begin: Inverse");
    }
}

void Save_Matrix_Result_As_File(FILE* fp){
    
    fprintf(fp,"Inversed Matrix:\n");
    int row, col;
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++)
            fprintf(fp, " %5.2f", I[row][col]);
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n\n");
    fclose(fp);

    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++)
            I[row][col]=0.0;
    }
}

void Init_Default(int problemsize, char* Initway, int max_num, int p)
{
    N = problemsize;
    Init = Initway;
    maxnum = max_num;
    PRINT = p;
}

int Read_Options(int argc, char** argv)
{
    char* prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch (*++ * argv) {
            case 'n':
                --argc;
                N = atoi(*++argv);
                break;
            case 'h':
                printf("\nHELP: try matinv -u \n\n");
                exit(0);
                break;
            case 'u':
                printf("\nUsage: matinv [-n problemsize]\n");
                printf("           [-D] show default values \n");
                printf("           [-h] help \n");
                printf("           [-I init_type] fast/rand \n");
                printf("           [-m maxnum] max random no \n");
                printf("           [-P print_switch] 0/1 \n");
                exit(0);
                break;
            case 'D':
                printf("\nDefault:  n         = %d ", N);
                printf("\n          Init      = rand");
                printf("\n          maxnum    = 5 ");
                printf("\n          P         = 0 \n\n");
                exit(0);
                break;
            case 'I':
                --argc;
                Init = *++argv;
                break;
            case 'm':
                --argc;
                maxnum = atoi(*++argv);
                break;
            case 'P':
                --argc;
                PRINT = atoi(*++argv);
                break;
            default:
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -u \n\n", prog);
                break;
            }
}
