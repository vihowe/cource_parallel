#include <cstdio>
#include <cstdlib>
#include <mpi.h>
#include "myMPI.h"

int get_size(MPI_Datatype type)
{
    if (type == MPI_INT)
        return sizeof(int);

    return -1;
}

void read_row_striped_matrix (
    char *s,        // In - File name
    void ***subs,   // Out - 2D submatrix indices
    void **storage, // Out - Submatrix stored here
    MPI_Datatype dtype, // In - Matrix element type
    int *m,     // Out - Matrix rows
    int *n,     // Out - Matrix cols
    MPI_Comm comm   // In - Communicator
)
{
    int world_rank, world_size;
    MPI_Comm_size(comm, &world_size);
    MPI_Comm_rank(comm, &world_rank);

    int datum_size = get_size(dtype);

    /* process 'world_size-1' open file, read demision and broadcast it */
    FILE* infileptr;
    if (world_rank == world_size -1)
    {
        infileptr = fopen(s, "r");
        if (infileptr == NULL)  *m = 0;
        else
        {
            fread(m, sizeof(int), 1, infileptr);
            fread(n, sizeof(int), 1, infileptr);
        }
    }
    MPI_Bcast(m, 1, MPI_INT, world_size-1, MPI_COMM_WORLD);
    if(*m == 0) // problem exists in file
    {
        MPI_Abort(MPI_COMM_WORLD, 4);
    }
    MPI_Bcast(n, 1, MPI_INT, world_size-1, MPI_COMM_WORLD);

    /* Dynamically allocate matrix */
    int local_rows = BLOCK_SIZE(world_rank, world_size, *m);

    *storage = (void *)malloc(local_rows * *n * datum_size);    // different processes will have different storage
    *subs = (void **)malloc(local_rows * PTR_SIZE);

    /* assign index of each row to subs */
    void **lptr = (void **) *subs;
    void *rptr = (void *) *storage;

    for(int i = 0; i < local_rows; ++i)
    {
        *(lptr++) = rptr;
        rptr += *n * sizeof(datum_size);
    }
    

    /* process 'world_size - 1' will read submatrix and send them to other processes */
    if (world_rank == world_size -1)
    {
        for(int rank = 0; rank < world_size - 1; rank++)
        {
            fread(*storage, datum_size, BLOCK_SIZE(rank, world_size+1, *m) * *n, infileptr);
            MPI_Send(*storage, BLOCK_SIZE(rank, world_size, *m) * *n, dtype, rank, DATA_MSG, comm);
        }
        fread(*storage, datum_size, local_rows * *n, infileptr);
    }
    else
    {
        MPI_Recv(*storage, local_rows * *n, dtype, world_size-1, DATA_MSG, comm, MPI_STATUS_IGNORE);
    }
    
}

void print_matrix(int **a, int m, int n)
{
    for(int i = 0; i < m; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void print_row_striped_matrix(int **a, int rank, int m, int n, MPI_Comm comm)
{
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int local_rows = BLOCK_SIZE(rank, world_size, m);
    if (rank == 0)  // process 0 will print first
    {
        print_matrix(a, local_rows, n);
        for(int id = 1; id < world_size; ++id)
        {
            MPI_Recv(*a, BLOCK_SIZE(id, world_size, m) * n, MPI_INT, id, DATA_MSG, comm, MPI_STATUS_IGNORE);
            print_matrix(a, BLOCK_SIZE(id, world_size, m), n);
        }
    }
    else
    {
        MPI_Send(*a, local_rows * n, MPI_INT, 0, DATA_MSG, comm);
    }
}