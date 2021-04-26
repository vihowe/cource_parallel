#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "myMPI.h"
#define _DEBUG

int get_size(MPI_Datatype type)
{
    if (type == MPI_INT)
        return sizeof(int);

    return -1;
}

void terminate(int id, char* error_msg)
{
    if (id == 0)
    {
        printf("Error: %s\n", error_msg);
        fflush(stdout);
    }
    MPI_Finalize();
    exit(-1);
}

void read_column_stripped_matrix(char* filename, void ***subs, void **storage, int *m,  int *n, MPI_Datatype dtype, MPI_Comm comm)
{
    int world_rank, world_size;
    MPI_Comm_rank(comm, &world_rank);
    MPI_Comm_size(comm, &world_size);


    int datum_size = get_size(dtype);

    // process p-1 is in charge of read data from file and send blocks to other processes
    FILE* infileptr;
    if (world_rank == world_size - 1)
    {
        infileptr = fopen(filename, "r");
        if (infileptr == NULL)  *m = 0;
        else
        {
            fread(m, sizeof(int), 1, infileptr);
            fread(n, sizeof(int), 1, infileptr);
        }
    }
    MPI_Bcast(m, 1, MPI_INT, world_size-1, comm);
    if (*m == 0)
        MPI_Abort(comm, OPEN_FILE_ERROR);

    MPI_Bcast(n, 1, MPI_INT, world_size-1, comm);
    int local_columns = BLOCK_SIZE(world_rank, world_size, *n);

    *storage = malloc(*m * local_columns * datum_size);
    *subs = (void**)malloc(*m * PTR_SIZE);

    void **lptr = (void**)*subs;
    void *rptr = *storage;
    for (int i = 0; i < *m; ++i)
    {
       *(lptr++)  = rptr;
       rptr += local_columns * datum_size;
    }

    void *buffer;
    if (world_rank == world_size - 1)
    {
        buffer = malloc(*n * datum_size);
    }
    int *send_count, *send_disp;
    create_mixed_xfer_arrays(world_rank, world_size, *n, &send_count, &send_disp);
    // {
    //     if (world_rank == 0)
    //     {
    //         printf("==============send_out, send_disp=============\n");
    //         for (int i = 0; i < world_size; ++i)
    //         {
    //             printf("%d %d\n", send_count[i], send_disp[i]);
    //         }
    //         printf("\n");
    //     }
    // }

    for (int i = 0; i < *m; ++i)
    {
        if (world_rank == world_size - 1)
        {
            fread(buffer, datum_size, *n, infileptr);
            // printf("===================line[%d]====================\n", i);
            // print_vect(buffer, *n, dtype);
            // printf("\n\n");
        }
        
        MPI_Scatterv(buffer, send_count, send_disp, dtype, (*storage)+ i * local_columns * datum_size,  local_columns, dtype, world_size-1, comm);

        // {
        //     printf("\n================storage in process[%d]==============\n", world_rank);
        //     for(int j = 0; j < local_columns; ++j)
        //     {
        //         printf("%d ", ((int *)(*storage))[i*local_columns+j]);
        //     }
        //     printf("\n=============\n");
        // }
    }
    free(send_count);
    free(send_disp);
    if (world_rank == world_size - 1)
        free(buffer);

}


void print_vect(void *buffer, int n, MPI_Datatype dtype);
void print_column_stripped_matrix(void **a, int m, int n, MPI_Datatype dtype, MPI_Comm comm)    // process 0 is in charge of receiving all data and print
{
    int world_rank, world_size;
    MPI_Comm_rank(comm, &world_rank);
    MPI_Comm_size(comm, &world_size);

    int datum_size = get_size(dtype);

    int *recv_count = (int *)malloc(sizeof(int) * world_size);
    int *recv_disp = (int *)malloc(sizeof(int) * world_size);
    create_mixed_xfer_arrays(world_rank, world_size, n, &recv_count, &recv_disp);
    // {
    //     printf("\n==================recv_count, recv_disp================\n");
    //     for(int i = 0; i < world_size; ++i)
    //     {
    //         printf("%d %d\n", recv_count[i], recv_disp[i]);
    //     }
    //     printf("\n==================\n");
    // }

    void *buffer;
    if (world_rank == 0)
        buffer = malloc(datum_size * n);    // a row of dat

    for (int i = 0; i < m; ++i)
    {
        MPI_Gatherv(a[i], BLOCK_SIZE(world_rank, world_size, n), dtype, buffer, recv_count, recv_disp, dtype, 0, comm);
        if (world_rank == 0)
        {
            print_vect(buffer, n, dtype);
            // print_vect(a[i], BLOCK_SIZE(0, world_size, n), dtype);
            printf("\n");
        }
    }
    free(recv_disp);
    free(recv_count);
    if (world_rank == 0)
        free(buffer);
}

void print_vect(void* buffer, int n, MPI_Datatype dtype)
{
    for(int i = 0; i < n; ++i)
    {
        if(dtype == MPI_INT)
        {
            printf("%4d ", ((int*)buffer)[i]);
        }
        else if(dtype == MPI_FLOAT)
        {
            printf("%6.3f", ((float*) buffer)[i]);
        }
    }

}

void create_mixed_xfer_arrays(int world_rank, int world_size, int n, int** send_count, int ** send_disp)
{
    *send_count = (int *)malloc(sizeof(int) * world_size);
    *send_disp = (int *)malloc(sizeof(int) * world_size);

    for(int i = 0; i < world_size; ++i)
    {
        (*send_count)[i] = BLOCK_SIZE(i, world_size, n);
    }
    (*send_disp)[0] = 0;
    for(int i = 1; i < world_size; ++i)
    {
        (*send_disp)[i] = (*send_disp)[i-1] + (*send_count)[i-1];
    }

}

void read_replicated_vector(char* filename, void** vec, MPI_Datatype dtype, int *n, MPI_Comm comm)
{
    int world_rank;
    int world_size;
    MPI_Comm_rank(comm, &world_rank);
    MPI_Comm_size(comm, &world_size);

    FILE* infileptr;
    if (world_rank == world_size - 1)
    {
        infileptr = fopen(filename, "r");
        if (infileptr == NULL)  *n = 0;
        else 
        {
            fread(n, sizeof(int), 1, infileptr);
            // printf("n=%d\n",*n);
        }
    }

    MPI_Bcast(n, 1, MPI_INT, world_size-1, comm);
    if (*n == 0)    terminate(0, "Cannot open vector file");

    int datum_size = get_size(dtype);
    *vec = malloc(datum_size * *n);
    
    if (world_rank == world_size - 1)
    {
        fread(*vec, datum_size, *n, infileptr);
        fclose(infileptr);
    }
    MPI_Bcast(*vec, *n, dtype, world_size-1, comm);
}


void create_uniform_xfer_arrays(int world_rank, int world_size, int n, int ** recv_in, int **recv_disp)
{
    *recv_in = malloc(sizeof(int) * world_size);
    *recv_disp = malloc(sizeof(int) * world_size);
    (*recv_in)[0] = BLOCK_SIZE(world_rank, world_size, n);
    (*recv_disp)[0] = 0;
    for (int i = 1; i < world_size; ++i)
    {
        (*recv_disp)[i] = (*recv_disp)[i-1] + (*recv_in)[i-1];
        (*recv_in)[i] = BLOCK_SIZE(world_rank, world_size, n);
    }

}


/**
 * Each process owns a block of vector
 * They all send that block to process 0
 * process 0 is responsible for printing
 */
void print_block_vector(void* vec, int n, MPI_Datatype dtype, MPI_Comm comm)
{
    int world_rank, world_size;
    MPI_Comm_rank(comm, &world_rank);
    MPI_Comm_size(comm, &world_size);
    int datum_size = get_size(dtype);

    int prompt; // just for notification
    MPI_Status status;   // result of receive
    if (world_rank == 0)
    {
        print_vect(vec, BLOCK_SIZE(world_rank, world_size, n), dtype);
        if (world_size > 1)
        {
            void* tmp = malloc(datum_size * BLOCK_SIZE(world_size-1, world_size, n));
            for (int p = 1; p < world_size; ++p)
            {
                MPI_Send(&prompt, 1, MPI_INT, p, PROMPT_MSG, comm);
                MPI_Recv(tmp, BLOCK_SIZE(p, world_size, n), dtype, p, RESPONSE_MSG, comm, &status);
                print_vect(tmp, BLOCK_SIZE(p, world_size, n), dtype);
            }
            free(tmp);
        }
    }
    else
    {
        MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, comm, &status);
        MPI_Send(vec, BLOCK_SIZE(world_rank, world_size, n), dtype, 0, RESPONSE_MSG, comm);
    }
}

void read_vector(char* filename, void** buffer, int* nv, MPI_Datatype dtype)
{
    FILE* infileptr = fopen(filename, "r");
    fread(nv, sizeof(int), 1, infileptr);

    int datum_size = get_size(dtype);

    *buffer = malloc(datum_size * *nv);
    fread(*buffer, datum_size * *nv, *nv, infileptr);
}
void read_matrix(char* filename, void*** matrix, void** storage, int* m, int* n, MPI_Datatype dtype)
{
    FILE* infileptr = fopen(filename, "r");
    fread(m, sizeof(int), 1, infileptr);
    fread(n, sizeof(int), 1, infileptr);
    int datum_size = get_size(dtype);
    *storage = malloc(datum_size * *m * *n);
    fread(*storage, datum_size, *m * *n, infileptr);

}

// int main(int argc, char ** argv)
// {
// //     int *send_count, *send_disp;
// //     int world_size = 3;
// //     int n = 11;

// //     create_mixed_xfer_arrays(0, world_size, n, &send_count, &send_disp);
// //     for(int i = 0; i < world_size; ++i)
// //     {
// //         printf("%d %d\n", send_count[i], send_disp[i]);
// //     }

//     FILE* outfileptr = fopen("vector_data", "w");
//     int n = 5;
//     int a[5] = {1, 2, 3, 4, 5};
//     fwrite(&n, sizeof(int), 1, outfileptr);
//     fwrite(a, sizeof(int), n, outfileptr);
//     fclose(outfileptr);

//     int* vec;

//     MPI_Init(NULL,NULL);
//     int world_rank, world_size;
//     MPI_Comm_size(MPI_COMM_WORLD, &world_size);
//     MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
//     read_replicated_vector("vector_data", (void **)&vec, MPI_INT, &n, MPI_COMM_WORLD);
//     if (world_rank == 0) print_vect(vec, n, MPI_INT);

//     MPI_Finalize();
//     return 0;

// }
