#define MAX_SIZE 4096
typedef double matrix[MAX_SIZE][MAX_SIZE];
void find_inverse(void);
void Init_Matrix(void);
void Print_Matrix(matrix M, char name[]);
void Init_Default(void);
int Read_Options(int, char**);