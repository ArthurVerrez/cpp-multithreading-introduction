#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <x86intrin.h>
//#include <intrin.h>
using namespace std;

constexpr int VV  = 4;

/*

inline __m128d sqrt_1(__m128d f)
    {
        return _mm_sqrt_sd(f, f);
    }

__m128d abs_1(__m128d val)
    {
        return _mm_castsi128_pd (_mm_srli_epi64 (_mm_slli_epi64 (_mm_castpd_si128 (val), 1), 1));
    }

__m128d norm(vector<double> U, int n){
    // Must compute the sum of root of the absolute value of each component.
    __m128d res = _mm_setzero_pd();
    for (int i = 0 ; i < n ; i+=VV){
        __m128d s0 = sqrt_1(abs_1(_mm_set_pd(U[i],U[i+1])));
        __m128d s1 = sqrt_1(abs_1(_mm_set_pd(U[i+2],U[i+3])));
        __m128d s2 = sqrt_1(abs_1(_mm_set_pd(U[i+4],U[i+5])));
        __m128d s3 = sqrt_1(abs_1(_mm_set_pd(U[i+6],U[i+7])));
        
        __m128d s1 = sqrt_1(abs_1(_mm_set_sd(U[i+1])));
        __m128d s2 = sqrt_1(abs_1(_mm_set_sd(U[i+2])));
        __m128d s3 = sqrt_1(abs_1(_mm_set_sd(U[i+3])));
        __m128d s4 = sqrt_1(abs_1(_mm_set_sd(U[i+4])));
        __m128d s5 = sqrt_1(abs_1(_mm_set_sd(U[i+5])));
        __m128d s6 = sqrt_1(abs_1(_mm_set_sd(U[i+6])));
        __m128d s7 = sqrt_1(abs_1(_mm_set_sd(U[i+7])));
        
        res = _mm_add_pd(res,s0);
        res = _mm_add_pd(res,s1);
        res = _mm_add_pd(res,s2);
        res = _mm_add_pd(res,s3);
        res = _mm_add_pd(res,s4);
        res = _mm_add_pd(res,s5);
        res = _mm_add_pd(res,s6);
        res = _mm_add_pd(res,s7);

        res  = _mm256_add_pd(_mm256_loadu_pd(sqrt(abs(U[i]))),res);
    }
    return res;
};
*/

void sse(float* U, int n)                                                                                                                                                                                        
{                      
  // We assume n % VV == 0.                                                                                                                                                                                        
  int nb_iters = n / VV;                                                                                                                                                                                         
  __m128* ptr = (__m128*)U;                                                                                                                                                                                      
 
  for (int i = 0; i < nb_iters; ++i, ++ptr, U += VV)                                                                                                                                                              
    _mm_store_ps(U, _mm_sqrt_ps(*ptr));                                                                                                                                                                          
}

int main()
{
    float* U;
    int n = 32;
    posix_memalign((void**)&U, 16,  n * sizeof(float));
    for (int i = 0; i < n; ++i){U[i] = 2.0;}

    sse(U,n);
    for (int i = 0; i < n; ++i){cout << U[i] << " ";}
    cout << endl;
}