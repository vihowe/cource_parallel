#include <mpi.h>
#include <iostream>
#include <string>
#include "myMPI.h"

#define ALIVE 1
#define DEAD 0
#define MPI_TYPE MPI_INT

using dtype=int;


void evolution(int id, int p, bool **a, int m, int n)
{
    // Only operate on part of rows
    int lowR = BLOCK_LOW(id, p, m);
    int highR = BLOCK_HIGH(id, p, m);
    int blocksize = highR - lowR + 1;

    bool ca[m][n];
    for(int i = 0; i < m; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            ca[i][j] = a[i][j];
        }
    }

    // int *temp1 = new int[n];
    // int *temp2 = new int[n];


    // if(lowR > 0)
    // {
    //     MPI_Send((void*)a[0], n, MPI_C_BOOL, id-1, 0, MPI_COMM_WORLD);
    //     MPI_Recv((void*)temp1, n, MPI_C_BOOL, id-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // }
    // if(highR < m-1)
    // {
    //     MPI_Send((void*)a[block_size-1], n, MPI_C_BOOL, id+1, 1, MPI_COMM_WORLD);
    //     MPI_Recv((void*)temp2, n, MPI_C_BOOL, id+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // }
    
    // bool access_temp1, access_temp2;
    // lowR > 0 ? access_temp1 = true : access_temp1 = false;
    // highR < n-1 ? access_temp2 = true : access_temp2 = false;

    for (int i = lowR; i <= highR; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            int num_lives = 0;
            
            if(i > 0)
            {
                if(j-1 >= 0 && a[i-1][j-1]) num_lives++;
                if(a[i-1][j]) num_lives++;
                if(j+1 < n && a[i-1][j+1]) num_lives++;
            }
            
            if (j-1 >= 0 && a[i][j-1]) num_lives++;
            if (j+1 < n && a[i][j+1]) num_lives++;

            if (i < m-1)
            {
                if(j-1 >= 0 && a[i+1][j-1]) num_lives++;
                if(a[i+1][j]) num_lives++;
                if(j+1 < n && a[i+1][j+1]) num_lives++;
            }

            if (a[i][j])
            {
                if(!(num_lives == 2 || num_lives == 3)) ca[i][j] = 0;
            }
            else if (!a[i][j] && num_lives == 3) ca[i][j] = 1;
        }
    }

    for(int i = lowR; i <= highR; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            a[i][j] = ca[i][j];
        }
    }

    for(int i = lowR; i <= highR; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }
}



void Evolve(int world_rank, int world_size, dtype** a, int m, int n)
{
    int local_rows = BLOCK_SIZE(world_rank, world_size, m);
    int lowR = BLOCK_LOW(world_rank, world_size, m);
    int highR = BLOCK_HIGH(world_rank, world_size, m);

    int p = 0, q = local_rows-1;
    if(world_size > 1)
    {
        if(world_rank != 0)
        {
            p = 1;
            q = local_rows;
        }
    }

    printf("In rank %d, p = %d, q = %d, local_rows = %d\n", world_rank, p, q, local_rows);

    int temp[local_rows][n];

    for(int i = p; i <= q; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            int num_lives = 0;
            if(j-1 >= 0 && a[i][j-1]) num_lives++;
            if(j+1 < n && a[i][j+1]) num_lives++;
            if(i > 0)
            {
                if(j-1 >= 0 && a[i-1][j-1]) num_lives++;
                if(a[i-1][j]) num_lives++;
                if(j+1 < n && a[i-1][j+1]) num_lives++;
            }
            if(i+1 < local_rows)
            {
                if(j-1 >= 0 && a[i+1][j-1]) num_lives++;
                if(a[i+1][j]) num_lives++;
                if(j+1 < n && a[i+1][j+1]) num_lives++;
            }

            if (a[i][j] == 0) num_lives == 3 ? temp[i][j] = 1 : temp[i][j] = 0;
            if (a[i][j] == 1) (num_lives == 2 || num_lives == 3) ? temp[i][j] = 1 : temp[i][j] = 0;
        }
    }

    for(int i = p; i < q; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            a[i][j] = temp[i][j];
        }
    }
}


int main(int argc, char** argv)
{
    // int j = atoi(argv[1]);
    // int k = atoi(argv[2]);

    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);

    

    int world_rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    
    dtype** a;
    dtype* storage;
    int m, n;

    if(world_rank == 0)
    {
        FILE* outfileptr = fopen((char *)"matrix.txt", "w+");
        int x = 5, y = 5;
        int z[x*y] = {1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1};
        fwrite(&x, sizeof(int), 1, outfileptr);
        fwrite(&y, sizeof(int), 1, outfileptr);
        fwrite(z, sizeof(int), x*y, outfileptr);
        fclose(outfileptr);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    read_row_striped_matrix((char*)"matrix.txt", (void***)&a, (void**)&storage, MPI_TYPE, &m, &n, MPI_COMM_WORLD);
    Evolve(world_rank, world_size, a, m, n);
    print_row_striped_matrix(a, m, n, MPI_COMM_WORLD);


    MPI_Finalize();
    return 0;
}