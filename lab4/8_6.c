// matrix-vector multiplication
// matrices are distributerd among the processors in block-column fasion
// vectors are replicated
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "myMPI.h"




int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    void **subs, *storage;
    int m, n;

    read_column_stripped_matrix("matrix.txt", (void***)&subs, (void**)&storage, &m, &n, MPI_INT, MPI_COMM_WORLD);
    // printf("%d %d", m,n);
    print_column_stripped_matrix(subs, m, n, MPI_INT, MPI_COMM_WORLD);

    MPI_Finalize();
    
    return 0;
}