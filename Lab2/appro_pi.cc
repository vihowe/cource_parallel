/**
 * author: vihowe
 */
#include <mpi.h>
#include <iostream>

#define LEN_INTERVAL 1e-6

double Rectangle_Area(double x)
{
    double y = 4 / (1 + x * x);
    return LEN_INTERVAL * y;
}

int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    double local_area = 0;
    for(double x = world_rank * LEN_INTERVAL; x < 1; x += LEN_INTERVAL * world_size)
    {
        local_area += Rectangle_Area(x);
    }
    double global_area;
    MPI_Reduce(&local_area, &global_area, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;

    if (world_rank == 0)
    {
        std::cout << "The approximation of PI is " << global_area << std::endl;
        std::cout << "Time consumed: " << elapsed_time << " ms" << std::endl;
    }
    
    MPI_Finalize();

}