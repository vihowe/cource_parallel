#include <cstdio>
#include <cstdlib>
#include <mpi.h>
#include "myMPI.h"
#define _DEBUG

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
    if((world_rank == 0 || world_rank == world_size - 1) && world_size > 1) local_rows += 1;
    else if(world_size > 1) local_rows += 2;

    // std::cout << "After overlapping, rank: " << world_rank << "  local_rows: " << local_rows << std::endl;
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
            int row_numbers = BLOCK_SIZE(rank, world_size, *m);
            if (rank == 0) row_numbers += 1;
            else row_numbers += 2;
            fread(*storage, datum_size, row_numbers * *n, infileptr);
            fseek(infileptr, -(datum_size * 2 * *n), SEEK_CUR);
            MPI_Send(*storage, row_numbers * *n, dtype, rank, DATA_MSG, comm);
            // std::cout << "rank(p-1) send to rank" << rank << " " << row_numbers << " rows" << std::endl;
        }
        fread(*storage, datum_size, local_rows * *n, infileptr);
        fclose(infileptr);
    }
    else
    {

        MPI_Recv(*storage, local_rows * *n, dtype, world_size-1, DATA_MSG, comm, MPI_STATUS_IGNORE);
        // std::cout << "rank" << world_rank << " received " << local_rows << " msg line successfully." << std::endl;
    }
}

void print_matrix(int **a, int m, int n, bool up, bool down)
{
    int i;
    up ? i = 1 : i = 0;
    down ? m -= 1 : m = m;
    for(; i < m; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }
    // std::cout << "one rank printing done" << std::endl;
}

void print_row_striped_matrix(int **a, int m, int n, MPI_Comm comm)
{
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Status status;
    int prompt;
    int local_rows = BLOCK_SIZE(world_rank, world_size, m);
    if (world_size > 1)
    {
        (world_rank == 0 || world_rank == world_size-1) ? local_rows += 1 : local_rows += 2;
    }

    if (world_rank == 0)
    {
        world_size > 1 ? print_matrix(a, local_rows, n, false, true) : print_matrix(a, local_rows, n, false, false);
        if(world_size > 1)
        {
            int max_block_size = BLOCK_SIZE(world_size-1, world_size, m) + 2;
            
            int *bstorage = (int*) malloc(sizeof(int) * max_block_size * n);
            int **b = (int **) malloc(PTR_SIZE * max_block_size);
            b[0] = bstorage;
            for (int i = 1; i < max_block_size; ++i)
            {
                b[i] = b[i-1] + n;
            }

            for (int i = 1; i < world_size; ++i)
            {
                int local_rows = BLOCK_SIZE(i, world_size, m);
                i == world_size-1 ? local_rows += 1 : local_rows += 2;
                MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG, comm);
                MPI_Recv(bstorage, local_rows * n, MPI_INT, i, RESPONSE_MSG, MPI_COMM_WORLD, &status);

                bool down = true;
                if (i == world_size - 1) down = false;

                print_matrix(b, local_rows, n, true, down);

                #ifdef _DEBUG
                {
                    std::cout << "DONE" << std::endl;
                }
                #endif
            }
            free(b);
            free(bstorage);
        }
    }
    else
    {
        MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, comm, &status);
        MPI_Send(*a, local_rows*n, MPI_INT, 0, RESPONSE_MSG, comm);
    }

}