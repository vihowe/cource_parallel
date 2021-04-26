// matrix-vector multiplication
// matrices are distributerd among the processors in block-column fasion
// vectors are replicated
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "myMPI.h"

typedef int dtype;
#define mpitype MPI_INT


int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    dtype **subs, *storage;
    int m, n;

    read_column_stripped_matrix("matrix_data", (void***)&subs, (void**)&storage, &m, &n, MPI_INT, MPI_COMM_WORLD);
    // printf("%d %d", m,n);
    // printf("\n==========storage in process[%d]=========\n", world_rank);
    // for (int i = 0; i < m; ++i)
    // {
    //     for(int j = 0; j < BLOCK_SIZE(world_rank, world_size, n); ++j)
    //     {
    //         printf("%d ", ((int**)subs)[i][j]);
    //     }
    //     // print_vect(subs[i], BLOCK_SIZE(world_rank, world_size, n), MPI_INT);
    //     printf("\n");
    // }
    // printf("==========");

    //
    if (world_rank == 0)
    {
        printf("\n==============matrix A===============\n");
    }
    print_column_stripped_matrix((void **)subs, m, n, MPI_INT, MPI_COMM_WORLD);

    dtype* vec;

    read_replicated_vector("vector_data", (void**)&vec, mpitype, &n, MPI_COMM_WORLD);
    if (world_rank == 0)
    {
        printf("\n==================vector B==============\n");
        print_vect(vec, n, mpitype);
    }
    int local_columns = BLOCK_SIZE(world_rank, world_size, n);
    dtype* c_part_out = (dtype*)malloc(sizeof(dtype) * m);

    int* send_count, *send_disp, *recv_count, *recv_disp;
    create_mixed_xfer_arrays(world_rank, world_size, n, &send_count, &send_disp);
    create_uniform_xfer_arrays(world_rank, world_size, n, &recv_count, &recv_disp);

    for (int i = 0; i < m; ++i)
    {
        c_part_out[i] = 0;
        for(int j = 0; j < local_columns; ++j)
        {
            c_part_out[i] += subs[i][j] * vec[send_disp[world_rank]+j];
        }
    }

    dtype* c_tmp = (dtype*)malloc(sizeof(dtype) * local_columns * world_size);

    MPI_Alltoallv(c_part_out, send_count, send_disp, mpitype, c_tmp, recv_count, recv_disp, mpitype, MPI_COMM_WORLD);

    dtype* c = (dtype*)malloc(sizeof(dtype) * local_columns);
    for (int i = 0; i < local_columns; ++i)
    {
        c[i] = 0;
        for (int j = 0; j < world_size; ++j)
        {
            c[i] += c_tmp[j * local_columns + i];
        }
    }

    // if (world_rank == 1)
    // {
    //     printf("\n=========partial result in process[%d], local_columns: %d======\n", world_rank, local_columns);
    //     print_vect(c_part_out, m, mpitype);
    // }

    if (world_rank == 0)
    {
        printf("\n==============result A * B =============\n");
    }
    print_block_vector((void*)c, n, mpitype, MPI_COMM_WORLD);


    MPI_Finalize();
    
    return 0;
}