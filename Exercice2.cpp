#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <x86intrin.h>
//#include <intrin.h>
using namespace std;

// Here, VV = 8;
constexpr int VV  = 8;
float sse(float* U, int n)                                                                                                                                                                                        
{                      
    // We assume n % VV == 0.                                                                                                                                                                                        
    int nb_iters = n / VV;                                                                                                                                                                                         
    __m128* ptr_1 = (__m128*)U;   
    __m128* ptr_2 = (__m128*)U;  
    ++ptr_2; 
                                                                                                                                                                                       
    float ans = 0.0;
    __m128 tmp;

    for (int i = 0; i < nb_iters; ++i,ptr_1+=2,ptr_2+=2, U += VV){

        tmp = _mm_add_ps(_mm_sqrt_ps(*ptr_1),_mm_sqrt_ps(*ptr_2));
        tmp =_mm_hadd_ps(tmp,tmp);
        tmp =_mm_hadd_ps(tmp,tmp);
        ans += tmp[0];

    };       
    return ans;                                                                                                                                                       
};

int main()
{
    float* U;
    int n = 32;
    posix_memalign((void**)&U, 16,  n * sizeof(float));
    for (int i = 0; i < n; ++i){U[i] = 2.0;}

    float ans = sse(U,n);
    cout << ans << endl;
}