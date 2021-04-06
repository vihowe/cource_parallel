/**
 * author: vihowe
 */
#include <mpi.h>
#include <iostream>
#include <string>
#include <cmath>

int cons_odd_pri(int start, int end)
{
    /* This function returns the number of times that two 
    consecutive odd numbers are both prime */

    bool is_pri(int number);    // return true if this number is prime
    
    int solutions = 0;
    int i = start % 2 == 1 ? start : start+1;
    for(; i+2 <= end; i +=2)
    {
        if (is_pri(i) && is_pri(i+2)) {
            solutions++;
        }
    }
    return solutions;
}

bool is_pri(int n)
{
    for(int i = 2; i <= (int)sqrt(n); ++i)
    {
        if (n % i == 0) 
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        std::cerr << "Please specify the range of sequence!";
        exit(-1);
    }
    int start = atoi(argv[1]);
    int end = atoi(argv[2]);
    if (start <= 0 || end <= 0)
    {
        std::cerr << "All should be positive";
        exit(-1);
    }

    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int workload = (end - start + 1) / world_size + 1;

    int p_start = world_rank * workload + 1;
    int p_end = p_start + workload -1;
    p_end = p_end > end ? end : p_end;

    // Maybe some consecutive odd numbers that accross the boundary are also both prime, we can overlap the boundary
    p_end = p_end + 2 > end ? end : p_end + 2;

    int local_count = cons_odd_pri(p_start, p_end);
    int global_count;
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time  = MPI_Wtime();
    double elapsed_time = end_time - start_time;
    if (world_rank == 0) {
        std::cout << "The number of two consectutive odd number which are both prime in [" << start << "," << end << "] is " << global_count << std::endl;

        std::cout << "Time consuming: " << elapsed_time << " ms" << std::endl;

    }
    MPI_Finalize();

}