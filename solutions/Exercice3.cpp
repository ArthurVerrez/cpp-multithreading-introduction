#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <pthread.h>
#include <time.h>
#include <x86intrin.h>
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

float ans = 0;
constexpr int VV  = 8;

struct thread_data{
    int thread_id; 
    float* u; 
    int indx_start; 
    int indx_end; 
};

#define NB_THREADS 128

struct thread_data thread_data_array[NB_THREADS];
int thread_i;
pthread_t thread_ptr[NB_THREADS];     
float rs[NB_THREADS];

__m128i minus1 = _mm_set1_epi32(-1);
__m128 absmask = _mm_castsi128_ps(_mm_srli_epi32(minus1, 1));

__m128 vecabs_and(__m128 v) {
  return _mm_and_ps(absmask, v);
}

void *computeNormVectoriel(void *threadarg){

    int indx_end, id, indx_start;
    struct thread_data *thread_pointer_data;

    thread_pointer_data = (struct thread_data *)threadarg;

    id = thread_pointer_data->thread_id;
    indx_start = thread_pointer_data->indx_start;
    indx_end = thread_pointer_data->indx_end;

    float* u = thread_pointer_data->u;

    // We assume n % VV == 0.                                                                                                                                                                                        
    int nb_iters = (indx_end - indx_start) / VV;                                                                                                                                                                                         
    __m128* ptr_1 = (__m128*)u;   
    __m128* ptr_2 = (__m128*)u;  
    ++ptr_2; 

    float ans_bis = 0.0;
    __m128 tmp;

    for (int i = indx_start; i < nb_iters + indx_start; ++i,ptr_1+=2,ptr_2+=2, u += VV){

        tmp = _mm_add_ps(_mm_sqrt_ps(vecabs_and(*ptr_1)),_mm_sqrt_ps(vecabs_and(*ptr_2)));
        tmp =_mm_hadd_ps(tmp,tmp);
        tmp =_mm_hadd_ps(tmp,tmp);
        ans_bis += tmp[0];

    };
    rs[id] = ans_bis;
    //ans += ans_bis;
    pthread_exit(NULL);
    return 0;
};

void *computeNormScalaire(void *threadarg){
    float tmp = 0.0;
    int indx_end, id, indx_start;
    struct thread_data *thread_pointer_data;

    thread_pointer_data = (struct thread_data *)threadarg;

    id = thread_pointer_data->thread_id;
    indx_start = thread_pointer_data->indx_start;
    indx_end = thread_pointer_data->indx_end;

    float* u = thread_pointer_data->u;
    
    for (int i = indx_start ; i < indx_end ; i++) tmp += sqrt(abs(u[i]));
    rs[id] = tmp;
    cout << "thread() : " << id << " Res: " << tmp << endl;
    //ans += tmp;
    pthread_exit(NULL);
    return 0;
};

float normPar(float* U, int n, int mode, int nb_threads)                                                                                                                                                                                        
{                 
    for (int i = 0 ; i < nb_threads ; i++){
        thread_i = i;

        thread_data_array[thread_i].thread_id = thread_i;
        thread_data_array[thread_i].u = U;
        thread_data_array[thread_i].indx_start = i*(n/nb_threads);
        thread_data_array[thread_i].indx_end = (i+1)*(n/nb_threads);
        //cout << "main() : creating thread, " << i << endl;
        if (mode < 1){pthread_create(&thread_ptr[thread_i], NULL, computeNormScalaire, (void *) &thread_data_array[thread_i]);} 
        else {pthread_create(&thread_ptr[thread_i], NULL, computeNormVectoriel, (void *) &thread_data_array[thread_i]);}
    }      

    for(int i=0;i<nb_threads;i++){
        pthread_join(thread_ptr[i], NULL);
    }
    for(int i=0;i<nb_threads;i++) ans += rs[i];
    return ans;                                                                                                                                        
};

/* 
If we use mode = 1, we must have: 
    n % 8*nb_thr == 0
*/

int main()
{
    float* U;
    int n = 64000000;
    posix_memalign((void**)&U, 16,  n * sizeof(float));
    for (int i = 0; i < n; ++i){U[i] = -1.0;}
    TIMER("Multi 128 threads with vectoriel");
    ans = normPar(U,n,1,NB_THREADS);
    cout << ans << endl;
}

/* Scalaire
16: 452ms
8: 352ms
4: 250ms
*/

/* Vectoriel
16: 165ms
8: 132ms
4: 110ms
*/