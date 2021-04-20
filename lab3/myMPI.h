
#define PTR_SIZE sizeof(void*)

#define BLOCK_LOW(id, p, n) ((id)*(n)/p)
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n) - 1)
#define BLOCK_OWNER(index, p, n) (((p)*((index)+1)-1)/(n))
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id)+1, p, n) - BLOCK_LOW(id, p, n))

#define DATA_MSG 0
#define PROMPT_MSG 1
#define RESPONSE_MSG 2

void read_row_striped_matrix (char *, void ***, void **, MPI_Datatype, int *, int *, MPI_Comm);

void print_row_striped_matrix(int **a, int m, int n, MPI_Comm comm);

void print_matrix(int **a, int m, int n, bool up, bool down);