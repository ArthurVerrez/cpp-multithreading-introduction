#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <x86intrin.h>
#include <time.h>
//#include <intrin.h>
using namespace std;

class Timer
{
public:
  Timer(const std::string& name)
    : name_ (name),
      start_ (clock())
    {
    }
  ~Timer()
    {
      double elapsed = (double(clock() - start_) / double(CLOCKS_PER_SEC));
      std::cout << name_ << ": " << int(elapsed * 1000) << "ms" << std::endl;
    }
private:
  std::string name_;
  clock_t start_;
}; 

#define TIMER(name) Timer timer__(name);

float sum8(__m256 x) {
    // hiQuad = ( x7, x6, x5, x4 )
    const __m128 hiQuad = _mm256_extractf128_ps(x, 1);
    // loQuad = ( x3, x2, x1, x0 )
    const __m128 loQuad = _mm256_castps256_ps128(x);
    // sumQuad = ( x3 + x7, x2 + x6, x1 + x5, x0 + x4 )
    const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);
    // loDual = ( -, -, x1 + x5, x0 + x4 )
    const __m128 loDual = sumQuad;
    // hiDual = ( -, -, x3 + x7, x2 + x6 )
    const __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);
    // sumDual = ( -, -, x1 + x3 + x5 + x7, x0 + x2 + x4 + x6 )
    const __m128 sumDual = _mm_add_ps(loDual, hiDual);
    // lo = ( -, -, -, x0 + x2 + x4 + x6 )
    const __m128 lo = sumDual;
    // hi = ( -, -, -, x1 + x3 + x5 + x7 )
    const __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);
    // sum = ( -, -, -, x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 )
    const __m128 sum = _mm_add_ss(lo, hi);
    return _mm_cvtss_f32(sum);
}

// Here, VV = 8;
constexpr int VV  = 8;
__m256i minus1 = _mm256_set1_epi32(-1);
__m256 absmask = _mm256_castsi256_ps(_mm256_srli_epi32(minus1, 1));

__m256 vecabs_and(__m256 v) {
	return _mm256_and_ps(absmask, v);
}

float sse(float* U, int n)                                                                                                                                                                                        
{                      
    // We assume n % VV == 0.                                                                                                                                                                                        
    int nb_iters = n / VV;                                                                                                                                                                                         
    __m256* ptr = (__m256*)U;    

    float ans = 0.0;
    __m256 tmp;

    for (int i = 0; i < nb_iters; ++i,++ptr, U += VV){
        tmp = _mm256_sqrt_ps(vecabs_and(*ptr));
        ans += sum8(tmp);
    };       
    return ans;                                                                                                                                                       
};

int main()
{
    float* U;
    int n = 64000000;
    posix_memalign((void**)&U, 16,  n * sizeof(float));
    for (int i = 0; i < n; ++i){U[i] = -1.0;}
    TIMER("SSE");
    float ans = sse(U,n);
    cout << ans << endl;
}