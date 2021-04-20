#include <cmath>
#include <iostream>
#include <chrono>
#include <ctime>

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


int main(int argc, char** argv)
{
    int n = atoi(argv[1]);

    bool marked[n-1] {0};

    auto start_time = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < n-1; ++i)
    {
        if(!marked[i])
        {
            int k = i+2;
            for(int j = i+k; j < n-1; j += k)
            {
                marked[j] = 1;
            }
        }
    }

    int cnt = 0;
    for (int i = 0; i < n-1; ++i)
    {
        if (!marked[i])
        {
            cnt++;
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << cnt << std::endl;
    
    std::cout << "Time consuming: " << duration.count() << "ms" <<  std::endl;
}