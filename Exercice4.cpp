#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <pthread.h>
#include <time.h>
#include <x86intrin.h>
using namespace std;

// ------------ Exercice 1 ------------ //
/*Commentaires sur norm:
On considère que l'adresse de U est alignée, on itère sur simplement sur les valeurs à la suite du pointeur et on utilise les fonctions de cmath.
*/

float norm(float* U, int n) {
	float accum = 0;
	for (int i = 0; i < n; i++) {
		accum += sqrt(abs(*(U + i)));
	}
	return accum;

}

// ------------------------------------ //

// ------------ Exercice 2 ------------ //
/*Commentaires sur vect_norm:

Comme l'addresse de U est alignée et que on suppose que n est un multiple de 8, deux choix s'offrent à nous:
    - utiliser un __m256 pour représenter chaque batch de 8 float
    - utiliser deux __m128 pour représenter chaque batch de 8 float

Dans chaque cas, on a besoin de pouvoir calculer une racine, une valeur absolue, et la somme des composants.
Pour la racine, ca ne pose pas de problèmes particuliers. Pour la somme, nous proposons deux méthodes différentes:
    - sommes croisées deux fois d'affilées a l'aide de _mm_hadd_ps
    - suite de shuffle et d'extraction du max
Finalement, pour la valeur absolue, on utilise un masque qui vient enlever les bits responsables des signes.

Finalement, pour la fonction en elle même, on utilise un pointeur ```__m256* ptr = (__m256*)U;```
(ou 2 en 128 :
```
__m128* ptr_1 = (__m128*)U;   
__m128* ptr_2 = (__m128*)U;  
++ptr_2; 
```
)
, qui sera décalé de 1 à chaque fois que U est décalé de 8. On peut ainsi parcourrir U et réaliser les différentes opérations.

*/

// _mm256

// abs 
__m256i minus1_256 = _mm256_set1_epi32(-1);
__m256 absmask_256 = _mm256_castsi256_ps(_mm256_srli_epi32(minus1_256, 1));

__m256 vecabs_and_256(__m256 v) {
	return _mm256_and_ps(absmask_256, v);
}
// sqrt: _mm256_sqrt_ps
// sum
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

// ----------------------------------
// _mm128

// abs 
__m128i minus1_128 = _mm_set1_epi32(-1);
__m128 absmask_128 = _mm_castsi128_ps(_mm_srli_epi32(minus1_128, 1));

__m128 vecabs_and_128(__m128 v) {
  return _mm_and_ps(absmask_128, v);
}
// sqrt: _mm_sqrt_ps
// sum: _mm_hadd_ps 2 fois à la suite
// ----------------------------------
constexpr int VV  = 8;

// _m256
float vect_norm_256(float* U, int n)                                                                                                                                                                                        
{                      
    // On considère que n % VV == 0.                                                                                                                                                                                        
    int nb_iters = n / VV;                                                                                                                                                                                         
    __m256* ptr = (__m256*)U;    

    float ans = 0.0;
    __m256 tmp;

    for (int i = 0; i < nb_iters; ++i,++ptr, U += VV){
        tmp = _mm256_sqrt_ps(vecabs_and_256(*ptr));
        ans += sum8(tmp);
    };       
    return ans;                                                                                                                                                       
};
// _m128
float vect_norm_128(float* U, int n)                                                                                                                                                                                        
{                      
    // On considère que n % VV == 0.                                                                                                                                                                                        
    int nb_iters = n / VV;                                                                                                                                                                                         
    __m128* ptr_1 = (__m128*)U;   
    __m128* ptr_2 = (__m128*)U;  
    ++ptr_2; 

    float ans = 0.0;
    __m128 tmp;

    for (int i = 0; i < nb_iters; ++i,ptr_1+=2,ptr_2+=2, U += VV){
        tmp = _mm_add_ps(_mm_sqrt_ps(vecabs_and_128(*ptr_1)),_mm_sqrt_ps(vecabs_and_128(*ptr_2)));
        tmp =_mm_hadd_ps(tmp,tmp);
        tmp =_mm_hadd_ps(tmp,tmp);
        ans += tmp[0];
    };       
    return ans;                                                                                                                                                       
};

// ------------------------------------ //

// ------------ Exercice 3 ------------ //
/*Commentaires sur normPar:

*/



// ------------------------------------ //





struct thread_data{
    int thread_id; 
    float* u; 
    int indx_start; 
    int indx_end; 
};

#define NB_THREADS 4 

struct thread_data thread_data_array[NB_THREADS];
int thread_i;
pthread_t thread_ptr[NB_THREADS];     
float rs[NB_THREADS];

float norm(float* U, int n){
    // Must compute the sum of root of the absolute value of each component.
    float tmp = 0;
    for (int i = 0 ; i < n ; i++){
        tmp += sqrt(abs(U[i]));
    }
    return tmp;
};

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

float* get_aligned_vec(int N) {
	srand(time(NULL));
	

	//float* a = (float*)_aligned_malloc(N * sizeof(float), 16); //Not the same on an unix machine
    float* a;
    posix_memalign((void**)&a, 16,  N * sizeof(float));

	for (int i = 0; i < N; i++) {
		*(a + i) = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	return a;
};

int main(int argc, char *argv[])
{

    cout << "You have entered " << argc 
         << " arguments:" << "\n"; 
  
    for (int i = 0; i < argc; ++i) 
        cout << argv[i] << "\n";

    float* U;
    int n = 64000000;
    posix_memalign((void**)&U, 16,  n * sizeof(float));
    for (int i = 0; i < n; ++i){U[i] = -1.0;}

    //TIMER("Version de base");
    //float ans1 = norm(U,n);

    TIMER("Version scalaire avec 4 threads");
    float ans2 = normPar(U,n,0,1);
    cout << ans2 << endl;
}
