
#define PTR_SIZE sizeof(void*)

#define BLOCK_LOW(id, p, n) ((id)*(n)/p)
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n) - 1)
#define BLOCK_OWNER(index, p, n) (((p)*((index)+1)-1)/(n))
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id)+1, p, n) - BLOCK_LOW(id, p, n))

#define DATA_MSG 0
#define PROMPT_MSG 1
#define RESPONSE_MSG 2

#define OPEN_FILE_ERROR -1

void read_column_stripped_matrix(char* filename, void ***subs, void **storage, int *m,  int *n, MPI_Datatype dtype, MPI_Comm comm);

void create_mixed_xfer_arrays(int world_rank, int world_size, int n, int** send_count, int **send_disp);
void print_column_stripped_matrix(void **a, int m, int n, MPI_Datatype dtype, MPI_Comm comm);
void print_vect(void *buffer, int n, MPI_Datatype dtype);

void read_replicated_vector(char* filename, void** vec, MPI_Datatype dtype, int *n, MPI_Comm comm);

void terminat(int id, char* error_msg);

void create_uniform_xfer_arrays(int world_rank, int world_size, int n, int **recv_count, int **recv_disp);

void print_block_vector(void *vec, int n, MPI_Datatype dtype, MPI_Comm comm);


void read_vector(char* filename, void** buffer, int* nv, MPI_Datatype dtype);

void read_matrix(char* filename, void*** matrix, void** storage, int* m, int* n, MPI_Datatype dtype);