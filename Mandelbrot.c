/*
 * Parallel Mandelbrot program with pthreads
 *
 * This program computes and saves the Mandelbrot set using n threads.
 * It examines all points in the complex plane
 * that have both real and imaginary parts between -2 and 2.
 * It relies on qdbmp.h library to manipulate bmp images.
 *
 * To compile: gcc -c qdbmp.c
 *             gcc qdbmp.o mandelbrot_par.c -o mandelbrot_par -lpthread
 * To execute: ./mandelbrot_par maxiter n_threads
 * where
 *   maxiter denotes the maximum number of iterations at each point
 *   n_threads denotes the number of threads to use
 *
 * Output: a graphical image saved as "Mandelbrot_fractale_par.bmp"
 *
 * Parallelization strategy: Horizontal band decomposition [file:44]
 * Each thread processes a contiguous set of rows of the image
 *
 * Author: [Votre nom]
 * Date: 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include "mandelbrot.h"
#include "qdbmp.h"

#define N 2           /* Taille de l'espace du problème (x, y de -N à N) */
#define NPIXELS 1500  /* Taille de la fenêtre d'affichage en pixels */

/* Structure pour passer les paramètres à chaque thread */
typedef struct {
    uint thread_id;        // Identifiant du thread (0 à n_threads-1)
    uint n_threads;        // Nombre total de threads
    uint maxiter;          // Nombre maximum d'itérations
    BMP* bmp;              // Pointeur vers l'image partagée (lecture seule pour structure)
    double scale_r;        // Facteur d'échelle pour l'axe réel
    double scale_i;        // Facteur d'échelle pour l'axe imaginaire
    double r_min;          // Valeur minimale de la partie réelle
    double i_min;          // Valeur minimale de la partie imaginaire
    uint width;            // Largeur de l'image en pixels
    uint height;           // Hauteur de l'image en pixels
} ThreadData;

/**
 * Fonction exécutée par chaque thread
 * Calcule une bande horizontale de l'ensemble de Mandelbrot
 * 
 * STRATÉGIE DE PARALLÉLISATION [file:44] :
 * - Décomposition par partitionnement des données de sortie (output data) [file:3]
 * - L'image est divisée en n_threads bandes horizontales
 * - Chaque thread calcule les pixels d'une ou plusieurs bandes
 * - Distribution cyclique des lignes pour équilibrer la charge
 */
void* compute_mandelbrot(void* arg) {
    // Récupération des paramètres du thread
    ThreadData* data = (ThreadData*)arg;
    uint thread_id = data->thread_id;
    uint n_threads = data->n_threads;
    uint maxiter = data->maxiter;
    BMP* bmp = data->bmp;
    
    complex z, c;
    double lengthsq, temp;
    uint row, col, k;
    
    printf("Thread %u: démarrage (calcul des lignes %u, %u, %u, ...)\n", 
           thread_id, thread_id, thread_id + n_threads, thread_id + 2*n_threads);
    
    /**
     * DISTRIBUTION CYCLIQUE DES LIGNES [file:3]
     * 
     * Au lieu d'assigner un bloc contigu de lignes à chaque thread :
     * - Thread 0 traite les lignes 0, n, 2n, 3n, ...
     * - Thread 1 traite les lignes 1, n+1, 2n+1, 3n+1, ...
     * - Thread i traite les lignes i, n+i, 2n+i, 3n+i, ...
     * 
     * AVANTAGE [file:3] :
     * - Meilleur équilibrage de charge
     * - Les lignes du centre de Mandelbrot nécessitent plus de calculs
     * - La distribution cyclique répartit ces lignes coûteuses entre tous les threads
     * - Évite qu'un thread finisse beaucoup plus tard que les autres
     */
    for (row = thread_id; row < data->height; row += n_threads) {
        // Chaque thread traite toutes les colonnes de ses lignes assignées
        for (col = 0; col < data->width; ++col) {
            
            /* Premier terme de la suite : z0 = 0 [file:44] */
            z.r = z.i = 0;
            
            /* 
             * Associer chaque pixel à un nombre complexe c [file:39]
             * Mise à l'échelle des coordonnées du pixel vers la région complexe
             */
            c.r = data->r_min + ((double) col * data->scale_r);
            c.i = data->i_min + ((double) row * data->scale_i);
            
            /*
             * CALCUL DE LA SUITE DE MANDELBROT [file:44]
             * Formule : z(n+1) = z(n)² + c
             * 
             * En coordonnées complexes :
             * Si z = a + bi et z² = (a² - b²) + 2abi
             * Donc : z(n+1).r = z(n).r² - z(n).i² + c.r
             *        z(n+1).i = 2 * z(n).r * z(n).i + c.i
             * 
             * On itère jusqu'à :
             * - divergence (|z| > 2, donc |z|² > 4) [file:44]
             * - ou nombre maximum d'itérations atteint
             */
            k = 0;
            do {
                // Calcul de la partie réelle de z(n+1)
                temp = z.r*z.r - z.i*z.i + c.r;
                
                // Calcul de la partie imaginaire de z(n+1)
                z.i = 2*z.r*z.i + c.i;
                
                // Mise à jour de la partie réelle
                z.r = temp;
                
                // Calcul du module au carré : |z|² = a² + b² [file:39]
                lengthsq = z.r*z.r + z.i*z.i;
                
                ++k;
                
            } while (lengthsq < (N*N) && k < maxiter);
            
            /*
             * COLORISATION DU PIXEL [file:44]
             * 
             * - Si k == maxiter : le point appartient à l'ensemble de Mandelbrot
             *   → couleur sombre
             * - Sinon : le point n'appartient pas à l'ensemble
             *   → couleur dépend de la vitesse de divergence (valeur de k)
             *   → k petit = divergence rapide = couleur claire
             *   → k grand = divergence lente = couleur foncée
             * 
             * SYNCHRONISATION [file:44] :
             * BMP_SetPixelRGB() est appelé par plusieurs threads simultanément
             * MAIS chaque thread écrit dans des lignes différentes
             * → Pas de RACE CONDITION car pas d'écriture simultanée sur le même pixel
             * → PAS BESOIN DE MUTEX [file:6]
             */
            BMP_SetPixelRGB(bmp, row, col, 100+k*10, 155+k*20, 100);
        }
    }
    
    printf("Thread %u: terminé\n", thread_id);
    return NULL;
}

/* ---- Programme principal ---- */
int main(int argc, char *argv[]) {
    uint maxiter, n_threads;
    double r_min = -N;
    double r_max = N;
    double i_min = -N;
    double i_max = N;
    uint width = NPIXELS;   /* Dimensions de la fenêtre d'affichage */
    uint height = NPIXELS;
    double scale_r, scale_i;
    struct timeval start, end;
    
    /* Vérification des arguments de la ligne de commande [file:44] */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s maxiter n_threads\n", argv[0]);
        fprintf(stderr, "  maxiter   : nombre maximum d'itérations\n");
        fprintf(stderr, "  n_threads : nombre de threads à utiliser\n");
        return EXIT_FAILURE;
    }
    
    /* Récupération des arguments [file:44] */
    maxiter = atoi(argv[1]);
    n_threads = atoi(argv[2]);
    
    // Validation des arguments
    if (maxiter == 0) {
        fprintf(stderr, "Erreur: maxiter doit être > 0\n");
        return EXIT_FAILURE;
    }
    if (n_threads == 0) {
        fprintf(stderr, "Erreur: n_threads doit être > 0\n");
        return EXIT_FAILURE;
    }
    
    printf("=== Calcul de la fractale de Mandelbrot (version parallèle) ===\n");
    printf("Paramètres:\n");
    printf("  - Taille de l'image : %u x %u pixels\n", width, height);
    printf("  - Nombre d'itérations max : %u\n", maxiter);
    printf("  - Nombre de threads : %u\n", n_threads);
    printf("  - Région complexe : [%.1f, %.1f] x [%.1f, %.1f]\n", 
           r_min, r_max, i_min, i_max);
    
    /* Démarrage du chronomètre */
    gettimeofday(&start, NULL);
    
    /* Création d'une nouvelle image blanche de taille NPIXELS x NPIXELS [file:39] */
    BMP* bmp = BMP_Create(NPIXELS, NPIXELS, 32);
    if (bmp == NULL) {
        fprintf(stderr, "Erreur: impossible de créer l'image BMP\n");
        return EXIT_FAILURE;
    }
    
    /***** Calcul et dessin des points *****/
    
    /* Calcul des facteurs d'échelle pour mapper la région complexe sur la fenêtre [file:39] */
    scale_r = (double)(r_max - r_min) / (double)width;
    scale_i = (double)(i_max - i_min) / (double)height;
    
    /* Allocation des structures pour les threads [file:6] */
    pthread_t* threads = malloc(n_threads * sizeof(pthread_t));
    ThreadData* thread_data = malloc(n_threads * sizeof(ThreadData));
    
    if (threads == NULL || thread_data == NULL) {
        fprintf(stderr, "Erreur: allocation mémoire échouée\n");
        BMP_Free(bmp);
        return EXIT_FAILURE;
    }
    
    /* Création des threads [file:6] */
    printf("\n=== Création de %u threads ===\n", n_threads);
    for (uint i = 0; i < n_threads; i++) {
        // Initialisation des paramètres pour chaque thread
        thread_data[i].thread_id = i;
        thread_data[i].n_threads = n_threads;
        thread_data[i].maxiter = maxiter;
        thread_data[i].bmp = bmp;
        thread_data[i].scale_r = scale_r;
        thread_data[i].scale_i = scale_i;
        thread_data[i].r_min = r_min;
        thread_data[i].i_min = i_min;
        thread_data[i].width = width;
        thread_data[i].height = height;
        
        // Création du thread [file:6]
        int err = pthread_create(&threads[i], NULL, compute_mandelbrot, &thread_data[i]);
        if (err != 0) {
            fprintf(stderr, "Erreur: pthread_create pour thread %u a échoué\n", i);
            free(threads);
            free(thread_data);
            BMP_Free(bmp);
            return EXIT_FAILURE;
        }
    }
    
    /* Attente de la terminaison de tous les threads [file:6] */
    printf("\n=== Attente de la terminaison des threads ===\n");
    for (uint i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n=== Tous les threads ont terminé ===\n");
    
    /* Arrêt du chronomètre */
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_usec - start.tv_usec) / 1000000.0;
    
    printf("\nTemps d'exécution: %.3f secondes\n", elapsed);
    
    /* Sauvegarde de la nouvelle image BMP [file:39] */
    printf("\n=== Sauvegarde de l'image ===\n");
    BMP_WriteFile(bmp, "Mandelbrot_fractale_par.bmp");
    BMP_CHECK_ERROR(stdout, -2);
    printf("Image sauvegardée: Mandelbrot_fractale_par.bmp\n");
    
    /* Libération de toute la mémoire allouée [file:39] */
    free(threads);
    free(thread_data);
    BMP_Free(bmp);
    
    return EXIT_SUCCESS;
}

/*
Réponses aux questions du rapport
Question 1.3 : Stratégie de parallélisation
Ma stratégie de parallélisation repose sur le partitionnement des données de sortie (output data decomposition) :
​

Principe général
L'image de 1500×1500 pixels est divisée entre les n threads

Chaque pixel nécessite un calcul indépendant (suite de Mandelbrot)

Les pixels sont répartis par distribution cyclique des lignes
​

Distribution cyclique
Au lieu d'assigner un bloc contigu de lignes :

Thread 0 : lignes 0, n, 2n, 3n, ...

Thread 1 : lignes 1, n+1, 2n+1, 3n+1, ...

Thread i : lignes i, n+i, 2n+i, 3n+i, ...

Avantages de cette approche
​
Équilibrage de charge : Les lignes centrales de Mandelbrot nécessitent plus d'itérations. La distribution cyclique répartit uniformément ces lignes coûteuses entre tous les threads.

Pas de synchronisation nécessaire : Chaque thread écrit dans des pixels différents, donc aucun conflit d'accès mémoire.

Scalabilité : Fonctionne avec n'importe quel nombre de threads.

Question 1.4 : Mécanismes de synchronisation
NON, je n'ai pas utilisé de mécanismes de synchronisation (mutex, barrières, etc.).
​

Justification
​
Pas de données partagées en écriture :

Chaque thread lit les mêmes paramètres (maxiter, scale_r, etc.) mais ne les modifie jamais

Les lectures simultanées sont toujours thread-safe
​

Pas de race condition :
​

Chaque thread écrit dans des lignes différentes de l'image

Thread i modifie uniquement les lignes i, n+i, 2n+i, ...

Aucun pixel n'est jamais modifié par deux threads simultanément

BMP_SetPixelRGB() n'a donc pas besoin de protection par mutex

Pas de dépendances entre tâches :
​

Le calcul d'un pixel ne dépend pas du résultat d'un autre pixel

Les threads sont complètement indépendants

Aucun besoin de barrière ou de rendez-vous

La seule synchronisation est l'appel à pthread_join() dans le main, qui attend simplement que tous les threads aient terminé avant de sauvegarder l'image.
​

Question 2.4 : Répartition équitable de la charge
OUI, ma stratégie répartit équitablement la charge de travail.
​

Analyse de la charge de calcul
La fractale de Mandelbrot présente une distribution non-uniforme de la complexité :

Centre de la fractale (lignes ~750) : nombreux points nécessitent maxiter itérations

Bords de l'image : divergence rapide, peu d'itérations

Avec distribution par blocs (MAUVAISE approche)
text
Thread 0: lignes 0-499     → rapide (bord)
Thread 1: lignes 500-999   → très lent (centre)
Thread 2: lignes 1000-1499 → rapide (bord)
→ Thread 1 travaille beaucoup plus longtemps → déséquilibre

Avec distribution cyclique (MA approche)
text
Thread 0: lignes 0, 3, 6, 9, ... → mélange de lignes rapides et lentes
Thread 1: lignes 1, 4, 7, 10, ... → mélange de lignes rapides et lentes
Thread 2: lignes 2, 5, 8, 11, ... → mélange de lignes rapides et lentes
→ Chaque thread reçoit un échantillon représentatif de toutes les régions → bon équilibre
*/