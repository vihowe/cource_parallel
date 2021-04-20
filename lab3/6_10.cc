/**
 * author: vihowe
 * data: 4/15/2021
 */

// Using function MPI_Send and MPI_Recv to implement your own version of
// MPI_Bcast. Compare the performance of your implementation with the real
// "MPI_Bcast" in your system's library

#include <mpi.h>
#include <iostream>

// #define _DEBUG

int My_MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    // Naive implementation: send the data from root to every other process
    int world_size, world_rank;
    MPI_Comm_size(comm, &world_size);
    MPI_Comm_rank(comm, &world_rank);

    if (world_rank == root) // this process is responsible for sending message
    {
        for(int i = 0; i < world_size; ++i)
        {
            if (i != root)
            {
                MPI_Send(buffer, count, datatype, i, 0, comm);
            }
        }
    }
    else
    {
        // this process need to receive the message from root
        MPI_Recv(buffer, count, datatype, root, 0, comm, MPI_STATUS_IGNORE);
    }
    MPI_Barrier(comm);
    return world_size;
}

int main(int argc, char** argv)
{
    int flag = atoi(argv[1]);

    MPI_Init(NULL, NULL);

    MPI_Barrier(MPI_COMM_WORLD);

    double start_time = MPI_Wtime();
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int msg = 0;
    for (int i = 0; i < 100000; ++i)
    {
        if (world_rank == 0)
        {
            msg++;
        }
        #ifdef _DEBUG
        {
            std::cout << "Before broadcast, world_rank[" << world_rank << "].msg=" << msg << std::endl;
        }
        #endif

        if (flag)
        {
            My_MPI_Bcast(&msg, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Bcast(&msg, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }
        

        #ifdef _DEBUG
        {
            std::cout << "After broadcast, world_rank[" << world_rank << "].msg=" << msg << std::endl;
        }
        #endif
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (world_rank == 0)
    {
        if (flag)
        {
            std::cout << "(My broadcast)";
        }
        else
        {
            std::cout << "(Lib's broadcast)";
        }
        std::cout << "Time consuming: " << (end_time - start_time) * 1000 << " ms" << std::endl;
    }

    MPI_Finalize();
    return 0;
    
}