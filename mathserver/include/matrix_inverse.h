#define MAX_SIZE 4096
typedef double matrix[MAX_SIZE][MAX_SIZE];
void find_inverse(void);
void Init_Matrix(FILE* fp);
void Init_Default(void);
int Read_Options(int, char**);
void Save_Matrix_Result_As_File(FILE* fp);