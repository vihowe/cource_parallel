/**
 * Write a manager/worker-style program to perform matrix-vector multiplication.
 * The manager process should read the vector from a file and distribute a copy
 * of it to all of the workers. Next, the manager should read the matrix from a
 * file and distribute rows of the matrix to the worker processes on demand.
 * For each row the manager sends a worker, it should receive in return an en
 * element of the solution vector. After all of the results have been received,
 * the manager should print the product vector to standard output.
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "myMPI.h"

typedef int dtype;
#define mpitype MPI_INT

#define VECTOR_LEN_MSG 0
#define MATRIX_M_MSG 1
#define MATRIX_N_MSG 2
#define REQUEST_MSG 3
#define RESULT_MSG 4
#define ROW_MSG 5
void manager(char* vector_filename, char* matrix_filename, MPI_Comm comm);
void worker(MPI_Comm comm);
int main(int argc, char ** argv)
{
    MPI_Init(NULL, NULL);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    char* matrix_filename = "./matrix_data";
    char* vector_filename = "./vector_data";

    if (world_rank == 0)
    {
        manager(vector_filename, matrix_filename, MPI_COMM_WORLD);
        // dtype* vec;
        // int m, n;
        // read_matrix(matrix_filename, NULL, (void**)&vec, &m, &n, mpitype);
        // for(int i = 0; i < m*n; ++i)
        // {
        //     printf("%4d", vec[i]);
        // }
    }
    else
    {
        worker(MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}


/**
 * process 0 is responsible for reading files and assign work to other processes
 */
void manager(char* vector_filename, char* matrix_filename, MPI_Comm comm)
{
    int world_size;
    MPI_Comm_size(comm, &world_size);

    dtype* vec;
    int nv;
    dtype* ms;
    dtype** matrix;
    int m, n;

    read_vector(vector_filename, (void**)&vec, &nv, mpitype);
    read_matrix(matrix_filename, (void***)&matrix, (void**)&ms, &m, &n, mpitype);
    printf("m=%d, n=%d, nv=%d\n",m, n, nv);
    if(m == 0 || n == 0 || nv == 0 || n != nv) MPI_Abort(comm, -9);
    MPI_Bcast(&n, 1, MPI_INT, 0, comm);
    MPI_Bcast(vec, n, MPI_INT, 0, comm);

    dtype* res = (dtype*)malloc(sizeof(dtype) * m); // store final result

    int terminated_cnt = 0;
    int assigned_rows = 0;
    int* assigned = (int*)malloc(sizeof(int) * world_size); // track which row the worker is assigned so that to store its result in a right position

    dtype tmp_res;    // to store the result from each worker

    MPI_Status status;
    int src, tag;
    do
    {
        MPI_Recv(&tmp_res, 1, mpitype, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        // printf("\n===========manager: src=%d, tag=%d, tmp_res=%d, =========\n", src, tag, tmp_res);

        if (tag == RESULT_MSG)
        {
            res[assigned[src]] = tmp_res;
        }

        // Assign more work or tell the work to terminate
        if (assigned_rows < m)
        {
            MPI_Send(&ms[assigned_rows*m], n, mpitype, src, ROW_MSG, comm);
            assigned[src] = assigned_rows;
            assigned_rows++;
        }
        else
        {
            MPI_Send(NULL, 0, MPI_INT, src, ROW_MSG, comm);
            terminated_cnt++;
        }

    } while (terminated_cnt < world_size - 1);

    for (int i = 0; i < m; ++i)
    {
        printf("%6d", res[i]);
    }
}

void worker(MPI_Comm comm)
{
    int world_rank, world_size;
    MPI_Comm_size(comm, &world_size);
    MPI_Comm_rank(comm, &world_rank);

    MPI_Request pending;    // handle for MPI_Isend
    MPI_Isend(NULL, 0, MPI_INT, 0, REQUEST_MSG, comm, &pending);

    int n;
    MPI_Bcast(&n, 1, MPI_INT, 0, comm);

    int *row = (int *)malloc(sizeof(int) * n);
    int *vec = (int *)malloc(sizeof(int) * n);
    MPI_Bcast(vec, n, MPI_INT, 0, comm);

    printf("\n==========process [%d]: n = %d===============\n",world_rank, n);
    
    MPI_Status status;
    int row_len;
    while (1)
    {
        MPI_Probe(0, ROW_MSG, comm, &status);
        MPI_Get_count(&status, MPI_INT, &row_len);
        printf("\n==========process [%d]: row_len = %d===============\n",world_rank, row_len);
        if (row_len == 0) break;

        MPI_Recv(row, n, mpitype, 0, ROW_MSG, comm, &status);
        int ret = 0;
        for (int i = 0; i < n; ++i)
        {
            ret += row[i] * vec[i];
        }
        MPI_Send(&ret, 1, MPI_INT, 0, RESULT_MSG, comm);


    }
}