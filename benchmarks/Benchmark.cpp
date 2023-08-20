#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <pthread.h>
#include <time.h>
#include <x86intrin.h>
#include <chrono>
using namespace std;

// ------------ Exercice 1 ------------ //
/*Commentaires sur norm:
On itère simplement sur les valeurs à la suite du pointeur et on utilise les fonctions de cmath.
*/

float norm(float *U, int n)
{
    double accum = 0;
    for (int i = 0; i < n; i++ , U++)
    {
        accum += sqrt(abs(*(U)));
    }
    return (float)accum;
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

__m256 vecabs_and_256(__m256 v)
{
    return _mm256_and_ps(absmask_256, v);
}
// sqrt: _mm256_sqrt_ps
// sum
float sum8(__m256 x)
{
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

__m128 vecabs_and_128(__m128 v)
{
    return _mm_and_ps(absmask_128, v);
}
// sqrt: _mm_sqrt_ps
// sum: _mm_hadd_ps 2 fois à la suite
// ----------------------------------
constexpr int VV = 8;

// _m256
float vect_norm_256(float *U, int n)
{
    // On considère que n % VV == 0.
    int nb_iters = n / VV;
    __m256 *ptr = (__m256 *)U;

    float ans = 0.0;
    __m256 tmp;

    for (int i = 0; i < nb_iters; ++i, ++ptr, U += VV)
    {
        tmp = _mm256_sqrt_ps(vecabs_and_256(*ptr));
        ans += sum8(tmp);
    };
    return ans;
};
// _m128
float vect_norm_128(float *U, int n)
{
    // On considère que n % VV == 0.
    int nb_iters = n / VV;
    __m128 *ptr_1 = (__m128 *)U;
    __m128 *ptr_2 = (__m128 *)U;
    ++ptr_2;

    float ans = 0.0;
    __m128 tmp;

    for (int i = 0; i < nb_iters; ++i, ptr_1 += 2, ptr_2 += 2, U += VV)
    {
        tmp = _mm_add_ps(_mm_sqrt_ps(vecabs_and_128(*ptr_1)), _mm_sqrt_ps(vecabs_and_128(*ptr_2)));
        tmp = _mm_hadd_ps(tmp, tmp);
        tmp = _mm_hadd_ps(tmp, tmp);
        ans += tmp[0];
    };
    return ans;
};

// ------------------------------------ //

// ------------ Exercice 3 ------------ //
/*Commentaires sur normPar:

*/

struct thread_data
{
    int thread_id;
    float *u;
    int indx_start;
    int indx_end;
    float rs;
};

void *computeNormScalaire(void *threadarg)
{
    float tmp = 0.0;
    int indx_end, id, indx_start;
    struct thread_data *thread_pointer_data;

    thread_pointer_data = (struct thread_data *)threadarg;

    id = thread_pointer_data->thread_id;
    indx_start = thread_pointer_data->indx_start;
    indx_end = thread_pointer_data->indx_end;

    float *u = thread_pointer_data->u;

    tmp = norm(u + indx_start, (indx_end - indx_start));
    thread_pointer_data->rs = tmp;
    //ans += tmp;
    pthread_exit(NULL);
    return 0;
};

void *computeNormVectoriel256(void *threadarg)
{

    int indx_end, id, indx_start;
    struct thread_data *thread_pointer_data;

    thread_pointer_data = (struct thread_data *)threadarg;

    id = thread_pointer_data->thread_id;
    indx_start = thread_pointer_data->indx_start;
    indx_end = thread_pointer_data->indx_end;

    float *u = thread_pointer_data->u;
    thread_pointer_data->rs = vect_norm_256(u + indx_start, (indx_end - indx_start));

    pthread_exit(NULL);
    return 0;
};

void *computeNormVectoriel128(void *threadarg)
{

    int indx_end, id, indx_start;
    struct thread_data *thread_pointer_data;

    thread_pointer_data = (struct thread_data *)threadarg;

    id = thread_pointer_data->thread_id;
    indx_start = thread_pointer_data->indx_start;
    indx_end = thread_pointer_data->indx_end;

    float *u = thread_pointer_data->u;
    thread_pointer_data->rs = vect_norm_128(u + indx_start, (indx_end - indx_start));

    pthread_exit(NULL);
    return 0;
};

float normPar(float *U, int n, int mode, int nb_threads)
{
    float ans = 0.;
    struct thread_data thread_data_array[nb_threads];
    pthread_t thread_ptr[nb_threads];

    for (int i = 0; i < nb_threads; i++)
    {

        thread_data_array[i].thread_id = i;
        thread_data_array[i].u = U;
        thread_data_array[i].rs = 0.0;
        thread_data_array[i].indx_start = i * (n / nb_threads);
        thread_data_array[i].indx_end = (i + 1) * (n / nb_threads);
        //cout << "main() : creating thread, " << i << endl;
        if (mode < 1)
        {
            pthread_create(&thread_ptr[i], NULL, computeNormScalaire, (void *)&thread_data_array[i]);
        }
        else if (mode <= 128)
        {
            pthread_create(&thread_ptr[i], NULL, computeNormVectoriel128, (void *)&thread_data_array[i]);
        } else {
            pthread_create(&thread_ptr[i], NULL, computeNormVectoriel256, (void *)&thread_data_array[i]);
        }
    }

    for (int i = 0; i < nb_threads; i++)
    {
        pthread_join(thread_ptr[i], NULL);
    }
    for (int i = 0; i < nb_threads; i++)
        ans += thread_data_array[i].rs;
    return ans;
};

// ------------------------------------ //



/* 
If we use mode = 1 or 256, we must have: 
    n % 8*nb_thr == 0
*/

// ------------ Exercice 4 ------------ //
/*Commentaires sur main:

- différence de précision (1 pour 1m environ)

*/

float *get_aligned_vec(int N)
{
    srand(time(NULL));

    //float* a = (float*)_aligned_malloc(N * sizeof(float), 16); //Not the same on an unix machine
    float *a;
    posix_memalign((void **)&a, 16, N * sizeof(float));

    for (int i = 0; i < N; i++)
    {
        *(a + i) = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    return a;
};

int main(int argc, char *argv[])
{

    float *U;
    int n;
    int nb_threads;
    int mode = 0;
    chrono::high_resolution_clock::time_point t1;
	chrono::high_resolution_clock::time_point t2;

    float res;
    /*
    We want to test for multiple n (32 * 1000, 10.000, 100.000, 1.000.000, 10.000.000, 100.000.000):

    -> norm 
    -> vectoriel 128 
    -> vectoriel 256 
    -> thread norm 
    -> thread vectoriel 128 
    -> thread vectoriel 256

    each 10x times. 
    We also want to test for nb_threads in {4, 8, 16, 32}
    */

    vector<int> sizes {1000,10000,100000,1000000,10000000,100000000};
    vector<int> threads {4,8,16,32};

    // type fonction,n,threads,time

    for (int q = 0 ; q < 10 ; q++){
        for (int z = 0 ; z < 6 ; z++){
            n = 32*sizes[z];
            U = get_aligned_vec(n);

            //norm 
            t1 = chrono::high_resolution_clock::now();
            res = norm(U, n);
            t2 = chrono::high_resolution_clock::now();
            cout << "norm," << n << "," << 1 << "," << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
            //vectoriel 128 
            t1 = chrono::high_resolution_clock::now();
            res = vect_norm_128(U,n);
            t2 = chrono::high_resolution_clock::now();
            cout << "vectoriel 128," << n << "," << 1 << "," << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
            //vectoriel 256 
            t1 = chrono::high_resolution_clock::now();
            res = vect_norm_256(U,n);
            t2 = chrono::high_resolution_clock::now();
            cout << "vectoriel 256," << n << "," << 1 << "," << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
            //thread norm 
            for (int c = 0 ; c < 4 ; c++){
                nb_threads = threads[c];
                t1 = chrono::high_resolution_clock::now();
                res = normPar(U,n,0,nb_threads);
                t2 = chrono::high_resolution_clock::now();
                cout << "norm," << n << "," << nb_threads << "," << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
            }
            //thread vectoriel 128 
            for (int c = 0 ; c < 4 ; c++){
                nb_threads = threads[c];
                t1 = chrono::high_resolution_clock::now();
                res = normPar(U,n,128,nb_threads);
                t2 = chrono::high_resolution_clock::now();
                cout << "vectoriel 128," << n << "," << nb_threads << "," << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
            }
            //thread vectoriel 256
            for (int c = 0 ; c < 4 ; c++){
                nb_threads = threads[c];
                t1 = chrono::high_resolution_clock::now();
                res = normPar(U,n,256,nb_threads);
                t2 = chrono::high_resolution_clock::now();
                cout << "vectoriel 256," << n << "," << nb_threads << "," << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
            }
        }
    }

}
