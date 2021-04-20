/**
 * author: vihowe
 * data: 4/15/2021
 */
#include "myMPI.h"
#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>

// #define _DEBUG

bool is_prime(int n)
{
    for (int i = 2; i <= (int)sqrt(n); ++i)
    {
        if (n % i == 0)
        {
            return false;
        }
    }
    return true;
}

std::vector<int> find_primes(int n)
{
    std::vector<int> v;
    for (int i = 2; i <= (int)sqrt(n); ++i)
    {
        if (is_prime(i))
        {
            v.push_back(i);
        }
    }
    return v;
}



int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cerr << "Please specify the range of array";
        exit(-1);
    }
    int n = atoi(argv[1]);
    bool marked[n-1]{0};

    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);

    double start_time = MPI_Wtime();

    /* Get the process' rank and the number of all processes */
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    std::vector<int> v;
    v = find_primes(n);     // let the process compute a list of primes alone, not by comm
    // if (world_rank == 0)
    // {
    //     v = find_primes(n);
    // }
    // MPI_Bcast(&v, v.size(), MPI_INT, 0, MPI_COMM_WORLD);
    std::vector<int> primes;
    for (int i = world_rank; i < v.size(); i += world_size)
    {
        primes.push_back(v[i]);
    }

    #ifdef _DEBUG
    {
        std::cout << "======vector: all primes between 2 and sqrt(n)=======" << std::endl;
        for (auto item : primes)
        {
            std::cout << item << " ";
        }
        std::cout << std::endl << "=============================================" << std::endl;
    }
    #endif

    for (int i = 0; i < primes.size(); ++i)
    {
        int k = primes[i];
        for (int j = k+1; j <= n; ++j)
        {
            if (j % k == 0) 
            {
                marked[j-2] = 1;
                // std::cout << "marked[" << j-2 << "]=" << marked[j-2] << std::endl;
            }
        }
    }

    #ifdef _DEBUG    
    {
        if (world_rank== 0)
        {
            std::cout << "=========the marked array==============" <<std::endl;
            for (int i = 0; i < n-1; ++i)
            {
                std::cout << marked[i] << " ";
            }
            std::cout << std::endl << "====================================--" << std::endl; 
        }
    }
    #endif

    /* Do the reduction, the number which are not marked(assigned true) by all processes are prime */
    bool ret[n-1]; 
    MPI_Reduce(marked, ret, n-1, MPI_C_BOOL, MPI_LOR, 0, MPI_COMM_WORLD);
    
    double end_time = MPI_Wtime();

    /* Display all primes */
    if (world_rank == 0)
    {
        printf("Time consuming: %4.2lf ms\n", (end_time - start_time)*1000);
        int count = 0;
        for (int i = 0; i < n-1; ++i)
        {
            if (!ret[i])
            {
                // std::cout << i+2 << " ";
                count++;
            }
        }
        std::cout << count << std::endl;
    }

    MPI_Finalize();
    return 0;
}