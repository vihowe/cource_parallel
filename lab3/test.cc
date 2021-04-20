#include <iostream>
#include <cstdlib>
// #include "myMPI.h"

void test(int **a, int ***pa)
{
    *a = (int*) malloc(16);  // 4 integer
    *pa = (int**) malloc(16);    // 2 ptr
    for (int i = 0; i < 4; ++i)
    {
        (*a)[i] = i;
    }

    (*pa)[0] = *a;
    for (int i = 1; i < 2; ++i)
    {
        (*pa)[i] = *a + 1;
    }

    // int *rptr = *a;
    // int **lptr = *pa;
    // int *rptr = (int *) *a;
    // int **lptr = (int **) *pa;

    // for(int i = 0; i < 2; ++i)
    // {
    //     *(lptr++) = rptr;
    //     rptr += 2;
    // }

}
void Evolve(int a[][5], int m, int n)
{
    

    int temp[m][n];

    for(int i = 0; i < m; ++i)
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
            if(i+1 < m)
            {
                if(j-1 >= 0 && a[i+1][j-1]) num_lives++;
                if(a[i+1][j]) num_lives++;
                if(j+1 < n && a[i+1][j+1]) num_lives++;
            }

            if (a[i][j] == 0) num_lives == 3 ? temp[i][j] = 1 : temp[i][j] = 0;
            if (a[i][j] == 1) (num_lives == 2 || num_lives == 3) ? temp[i][j] = 1 : temp[i][j] = 0;
            // std::cout << temp[i][j] << " ";
        }
        // std::cout << std::endl;
    }

    for(int i = 0; i < 5; ++i)
    {
        for(int j = 0; j < n; ++j)
        { 
            a[i][j] = temp[i][j];
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }
}
int main()
{
    // int a = BLOCK_HIGH(0, 2, 10);
    // std::cout << sizeof(int) << sizeof(void) << std::endl;
    // int *a = NULL;
    // int **pa = NULL;
    // test(&a, &pa);

    // std::cout << "Print from a" << std::endl;
    // for(int i = 0; i < 4; ++i)
    // {
    //     std::cout << a[i] << " ";
    // }
    // std::cout << std::endl << "Print from pa" << std::endl;
    // for(int i = 0; i < 2; ++i)
    // {
    //     for(int j = 0; j < 2; ++j)
    //     {
    //         std::cout << pa[i][j] << " ";
    //     }
    // }

    // FILE* outfileptr = fopen((char*)"matrix.txt", "w+");
    // int m = 0;
    // int b = 1;
    // // fscanf(infileptr, "%d", &m);
    // fwrite(&m, sizeof(int), 1, outfileptr);
    // fclose(outfileptr);

    // FILE* infileptr = fopen((char*)"matrix.txt", "r");
    // fread(&b, sizeof(int), 1, infileptr);

    // std::cout << b;

    // int a[3][4];
    // for (int i = 0; i < 3; ++i)
    // {
    //     for (int j = 0; j < 4; ++j)
    //     {
    //         std::cout << a[i][j] << " ";
    //     }
    //     std::cout << std::endl;

    // }
    int a[5][5] = {
        {1, 0, 0, 1, 1,},
        {0, 0, 0, 1, 1,},
        {0, 0, 0, 0, 1,},
        {1, 1, 1, 0, 1,},
        {0, 0, 0, 0, 1,},
    };
    Evolve(a, 5, 5);

}