/**
 * A perfect number is a positive integer number whose value is equal to the 
 * sum of all its positive factors, excluding itself. The first two perfect
 * numbers are 6 and 28:
 *      6 = 1 + 2 + 3
 *      28 = 1 + 2 + 4 + 7 + 14
 * The Greek mathematician Euclid(c. 300BCE) showed that if 2^n - 1 is prime,
 * then (2^n - 1)*2^(n-1) is a perfect number. For example, 2^2 - 1 = 3 is prime,
 * so (2^2 - 1)*2^(2-1) = 6 is a perfect number.
 * Write a parallel program to find the first eight perfect numbers
 */
#include <mpi.h>
// #include "myMPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define REQUEST_MSG 0
#define RESPONSE_MSG 1
#define DATA_MSG 2

// manager is responsible for generating 2^n-1
// worker is responsible for verifying where it is a prime

int is_prime(int n)
{
    if (n == 1) return 0;
    for (int i = 2; i < (int)sqrt(n); ++i)
    {
        if (n % i == 0)
            return 0;
    }
    return 1;
}
void manager(int n, MPI_Comm comm);
void worker(MPI_Comm comm);

int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int n = 8;  // we need to find n perfect numbers

    if (world_rank == 0)
    {
        manager(n, MPI_COMM_WORLD);
    }
    else
    {
        worker(MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;

}


void manager(int n, MPI_Comm comm)
{
    int world_size;
    MPI_Comm_size(comm, &world_size);

    long* ret = (long *)malloc(sizeof(long) * n);
    int ret_cnt = 0;

    int tmp_idx = 1;

    MPI_Status status;
    long tmp_ret;
    int src, tag;
    do{
        MPI_Recv(&tmp_ret, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);

        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;

        if (tag == RESPONSE_MSG)
        {
            if (tmp_ret)
            {
                ret[ret_cnt++] = (pow(2, tmp_ret) - 1) * pow(2, tmp_ret - 1);
            }
        }
        if (ret_cnt == n)   // done, send msg to all workers to stop
        {
            for (int p = 1; p < world_size; ++p)
            {
                MPI_Send(NULL, 0, MPI_INT, p, DATA_MSG, comm);
            }
        }
        else
        {
            MPI_Send(&tmp_idx, 1, MPI_INT, src, DATA_MSG, comm);
            tmp_idx++;
        }
    }while(ret_cnt < n);

    for (int i = 0; i < n; ++i)
    {
        printf("%ld  ", ret[i]);
    }
   
}


void worker(MPI_Comm comm)
{
    int world_rank, world_size;
    MPI_Comm_rank(comm, &world_rank);
    MPI_Comm_size(comm, &world_size);

    MPI_Request pending; // handler for MPI_Isend
    int ret;
    MPI_Isend(NULL, 0, MPI_INT, 0, REQUEST_MSG, comm, &pending);

    MPI_Status status;  // status for receiving
    int n;
    while (1)
    {
        MPI_Probe(0, DATA_MSG, comm, &status);
        MPI_Get_count(&status, MPI_INT, &n);
        if (n == 0)
        {
            printf("\n======process[%d]: done========\n", world_rank);
            break;
        }
        MPI_Recv(&n, 1, MPI_INT, 0, DATA_MSG, comm, &status);
        if (is_prime(pow(2,n)-1))
        {
            ret = n;
        }
        else
        {
            ret = 0;
        }
        MPI_Send(&ret, 1, MPI_INT, 0, RESPONSE_MSG, comm);
    }
}