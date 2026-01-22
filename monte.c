/*
 * Programme de calcul d'intégrale par méthode de Monte Carlo - Version parallèle
 * 
 * Objectif: Estimer l'intégrale I = ∫₀¹ x² dx = 1/3 ≈ 0.333333
 * 
 * Méthode de Monte Carlo:
 * - Générer N points aléatoires x dans [0,1]
 * - Calculer la moyenne de x² sur ces N points
 * - Cette moyenne converge vers l'intégrale quand N → ∞
 * 
 * Compilation: gcc monte.c -o monte -lpthread -lm
 * Utilisation: ./monte N n_threads
 *   N         : nombre de points à générer (ex: 10000000)
 *   n_threads : nombre de threads à utiliser (ex: 4)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Structure pour passer les paramètres à chaque thread
typedef struct {
    int thread_id;        // Identifiant du thread (0 à n_threads-1)
    int n_threads;        // Nombre total de threads
    long N;               // Nombre total de points à générer
    unsigned int seed;    // Graine pour le générateur aléatoire (thread-safe)
    double partial_sum;   // Somme partielle calculée par ce thread (RÉSULTAT)
} ThreadData;

/**
 * Fonction exécutée par chaque thread
 * 
 * STRATÉGIE DE PARALLÉLISATION:
 * - Décomposition par partitionnement des données (data decomposition)
 * - Distribution CYCLIQUE des points entre les threads
 * - Thread i génère les points i, n+i, 2n+i, 3n+i, ...
 * - Chaque thread calcule sa somme partielle localement (pas de variable partagée)
 * - Pas besoin de mutex car chaque thread écrit uniquement dans sa propre structure
 */
void* compute_monte_carlo(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    int thread_id = data->thread_id;
    int n_threads = data->n_threads;
    long N = data->N;
    unsigned int seed = data->seed;  // Chaque thread a sa propre graine
    
    double local_sum = 0.0;  // Somme locale (privée au thread)
    
    printf("Thread %d: démarrage (calcul des points %d, %d, %d, ...)\n", 
           thread_id, thread_id, thread_id + n_threads, thread_id + 2*n_threads);
    
    /*
     * DISTRIBUTION CYCLIQUE [TD: voir cours sur data decomposition]
     * - Thread 0: points 0, n, 2n, 3n, ...
     * - Thread 1: points 1, n+1, 2n+1, 3n+1, ...
     * - Thread i: points i, n+i, 2n+i, 3n+i, ...
     * 
     * AVANTAGE: Équilibrage automatique de la charge
     */
    for (long i = thread_id; i < N; i += n_threads) {
        /*
         * Génération d'un nombre aléatoire dans [0, 1]
         * IMPORTANT: Utilisation de rand_r(&seed) pour thread-safety
         * rand() standard n'est PAS thread-safe !
         */
        double x = (double)rand_r(&seed) / RAND_MAX;
        
        // Calcul de x² et accumulation dans la somme locale
        local_sum += x * x;
    }
    
    // Stockage du résultat dans la structure (sera récupéré par le main)
    data->partial_sum = local_sum;
    
    printf("Thread %d: terminé (somme partielle = %f)\n", thread_id, local_sum);
    
    return NULL;
}

int main(int argc, char* argv[]) {
    long N;
    int n_threads;
    struct timeval start, end;
    
    // === 1. VÉRIFICATION DES ARGUMENTS ===
    if (argc != 3) {
        fprintf(stderr, "Usage: %s N n_threads\n", argv[0]);
        fprintf(stderr, "  N         : nombre de points à générer\n");
        fprintf(stderr, "  n_threads : nombre de threads à utiliser\n");
        fprintf(stderr, "Exemple: %s 10000000 4\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // === 2. RÉCUPÉRATION DES ARGUMENTS ===
    N = atol(argv[1]);           // atol pour long
    n_threads = atoi(argv[2]);
    
    // Validation
    if (N <= 0) {
        fprintf(stderr, "Erreur: N doit être > 0\n");
        return EXIT_FAILURE;
    }
    if (n_threads <= 0) {
        fprintf(stderr, "Erreur: n_threads doit être > 0\n");
        return EXIT_FAILURE;
    }
    
    printf("=== Calcul d'intégrale par Monte Carlo (version parallèle) ===\n");
    printf("Intégrale: ∫₀¹ x² dx = 1/3 ≈ 0.333333\n");
    printf("Paramètres:\n");
    printf("  - Nombre de points: %ld\n", N);
    printf("  - Nombre de threads: %d\n", n_threads);
    printf("  - Points par thread: ~%ld\n", N / n_threads);
    
    // === 3. DÉMARRAGE DU CHRONOMÈTRE ===
    gettimeofday(&start, NULL);
    
    // === 4. ALLOCATION MÉMOIRE ===
    pthread_t* threads = malloc(n_threads * sizeof(pthread_t));
    ThreadData* thread_data = malloc(n_threads * sizeof(ThreadData));
    
    if (threads == NULL || thread_data == NULL) {
        fprintf(stderr, "Erreur: allocation mémoire échouée\n");
        return EXIT_FAILURE;
    }
    
    // === 5. CRÉATION DES THREADS ===
    printf("\n=== Création de %d threads ===\n", n_threads);
    for (int i = 0; i < n_threads; i++) {
        // Initialisation des paramètres pour chaque thread
        thread_data[i].thread_id = i;
        thread_data[i].n_threads = n_threads;
        thread_data[i].N = N;
        
        /*
         * IMPORTANT: Chaque thread a une graine différente pour rand_r()
         * Sinon tous généreraient la même séquence de nombres aléatoires !
         */
        thread_data[i].seed = (unsigned int)(time(NULL) + i);
        thread_data[i].partial_sum = 0.0;
        
        // Création du thread
        int err = pthread_create(&threads[i], NULL, compute_monte_carlo, &thread_data[i]);
        if (err != 0) {
            fprintf(stderr, "Erreur: pthread_create thread %d échoué\n", i);
            free(threads);
            free(thread_data);
            return EXIT_FAILURE;
        }
    }
    
    // === 6. ATTENTE DE LA TERMINAISON ===
    printf("\n=== Attente de la terminaison des threads ===\n");
    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n=== Tous les threads ont terminé ===\n");
    
    // === 7. AGRÉGATION DES RÉSULTATS ===
    /*
     * SYNCHRONISATION: Agrégation séquentielle après pthread_join
     * - Chaque thread a calculé sa somme partielle dans thread_data[i].partial_sum
     * - Le main additionne ces sommes partielles (pas de race condition)
     * - Cette phase est séquentielle mais très courte (O(n_threads))
     */
    double total_sum = 0.0;
    for (int i = 0; i < n_threads; i++) {
        total_sum += thread_data[i].partial_sum;
        printf("Thread %d: somme partielle = %f\n", i, thread_data[i].partial_sum);
    }
    
    // Calcul du résultat final (moyenne)
    double result = total_sum / N;
    
    // === 8. ARRÊT DU CHRONOMÈTRE ===
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_usec - start.tv_usec) / 1000000.0;
    
    // === 9. AFFICHAGE DES RÉSULTATS ===
    printf("\n=== Résultats ===\n");
    printf("Intégrale estimée = %.6f\n", result);
    printf("Valeur théorique  = %.6f (1/3)\n", 1.0/3.0);
    printf("Erreur absolue    = %.6f\n", fabs(result - 1.0/3.0));
    printf("Temps d'exécution = %.3f secondes\n", elapsed);
    
    // === 10. LIBÉRATION MÉMOIRE ===
    free(threads);
    free(thread_data);
    
    return EXIT_SUCCESS;
}