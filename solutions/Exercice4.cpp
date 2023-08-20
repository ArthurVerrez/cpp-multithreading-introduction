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
    cout.precision(13);
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
        cout << "main() : creating thread, " << i << endl;
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

// ------------ Exercice 4 ------------ //
/*Commentaires sur main:

- différence de précision (1 pour 1million environ)
- benchmark réalisé sur 10 expériences, pour nb_threads dans {4,8,16,32} et des vecteurs de tailles 
    32*{1000,10000,100000,1000000,10000000,100000000}
- benchmark aussi réalisé sous Java, les différentes fonctions utilisées sont présentes à la fin du fichier.

Les résultats sont présentés en pièce-jointes (pdf).

*/

// ------------ Exercice 5 ------------ //
/*Comment adapter vect_norm pour:
    1. une addresse U non alignée
    2. n quelconque

1.
Si n est toujours un multiple de 8, il est possible d'aligner U et de réutiliser la même fonction (que ce soit avec 
```std::aligned_alloc```, ```posix_memalign``` ou plus récemment ```std::align_val_t```. 
)

Cependant, si l'on se refuse à aligner U, car l'on considère que cette opération prend trop de temps, il devient très
difficile d'utiliser de la vectorisation. En effet, de nombreux problèmes peuvent advenir:
* Le programme sera plus lent
* L'OS peut crash
* Le programme peut renvoyer des résultats incorrects.

De manière générale, il est crucial d'aligner les adresses des variables utilisées pour faire de la vectorisation.

2. 
On peut utiliser 2 solutions (en fonction de ce qui est le mieux pour l'allocation mémoire):
    - padder U avec des 0. jusqu'à ce que n soit multiple de 8 (au plus 7 ajouts)
    - retirer les n%8 <= 7 dernières valeurs de U, puis les rajouter en utilisant norm.

*/

int main(int argc, char *argv[])
{

    float *U;
    int n;
    int nb_threads;
    int mode = 0;
    chrono::high_resolution_clock::time_point t1;
	chrono::high_resolution_clock::time_point t2;

    float res1, res2;

    cout << "You have entered " << argc
         << " arguments:"
         << "\n";

    for (int i = 0; i < argc; ++i)
        cout << "Argument " << i  << " -> " << argv[i] << "\n";

    // -----------------------

    if(argc>1) {n=atoi(argv[1]);} else {n = 64000000;}
    if(argc>2) {nb_threads=atoi(argv[2]);} else {nb_threads = 4;}
    cout << " ---------------------------------------------- " << endl;
    // -----------------------

    U = get_aligned_vec(n);
    cout << "U : " << " ";
    for (int i = 0 ; i < 10 ; i++){
        cout <<*(U+i) << " ";
    }
    cout << " ... ";
    cout << endl;
    cout << " ---------------------------------------------- " << endl;
    // Computation for scalaire and vectoriel + threads:
    cout << "Function: norm" << endl;
    t1 = chrono::high_resolution_clock::now();
	res1 = norm(U, n);
	t2 = chrono::high_resolution_clock::now();
    cout << "Results: " << res1 << endl;
    cout << "duration:" << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()/1000 << "ms" << endl;
    cout << " ---------------------------------------------- " << endl;

    // Computation for scalaire and vectoriel + threads:
    cout << "Function: normPar" << endl;
    cout << "with nb_threads = " << nb_threads << endl;

    t1 = chrono::high_resolution_clock::now();
	res2 = normPar(U,n,1,nb_threads);
	t2 = chrono::high_resolution_clock::now();

    cout << "Results: " << res2 << endl;
    cout << "duration:" << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()/1000 << "ms" << endl;
    cout << " ---------------------------------------------- " << endl;

}

/*
Benchmark Java:

JAVA-Traditional for-loop

for (int i = 0; i < N; i++) {
    res += (float)Math.sqrt(Math.abs(U.get(i)));
}

------------------------------

JAVA-Traditional iterator-loop

while (iterator.hasNext()) {
    res += (float)Math.sqrt(Math.abs(iterator.next()));
}

------------------------------

JAVA-for-loop object

for (Float item : U) {
    res += (float)Math.sqrt(Math.abs(item));
}

------------------------------

JAVA-for-each lambda

U.forEach(val -> Math.sqrt(Math.abs(val)));
res = (float)U.stream()
.mapToDouble(a -> a)
.sum();

------------------------------

JAVA-parallel stream

U.parallelStream().forEach(val -> Math.sqrt(Math.abs(val)));
res = (float)U.parallelStream()
.mapToDouble(a -> a)
.sum();

------------------------------

*/