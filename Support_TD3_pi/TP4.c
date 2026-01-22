#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    long nb_points_a_generer;   
	long nb_points_dans_cercle;
} donnees_thread_t;


void *fonction_thread(void *arg) {
    donnees_thread_t *donnees = (donnees_thread_t *)arg;
    long compteur = 0;

    unsigned int graine = (unsigned int)time(NULL) ^ (unsigned int)(size_t)arg;

    for (long i = 0; i < donnees->nb_points_a_generer; i++) {
        double x = (double)-rand_r(&graine) / (double)RAND_MAX;
        double y = (double)rand_r(&graine) / (double)RAND_MAX;

        if (x * x + y * y <= 1.0) {
            compteur++;
        }
    }

    donnees->nb_points_dans_cercle = compteur;
    return NULL;
}

int main(void)
{

    long nb_points_total = 1000000;
    int nb_threads = 10;

    pthread_t *threads = malloc(nb_threads * sizeof(pthread_t));
    donnees_thread_t *donnees = malloc(nb_threads * sizeof(donnees_thread_t));

	long nb_points_par_thread = nb_points_total / nb_threads;
	long reste = nb_points_total % nb_threads;
	clock_t tmp_beg = clock();

    // création des threads
    for (int i = 0; i < nb_threads; i++)
	{
        donnees[i].nb_points_a_generer = nb_points_par_thread;
        if (i == 0)
		{
            donnees[i].nb_points_a_generer += reste;
        }
        donnees[i].nb_points_dans_cercle = 0;

        pthread_create(&threads[i], NULL, fonction_thread, &donnees[i]);
	}

    long total_dans_cercle = 0;
    for (int i = 0; i < nb_threads; i++) {
        pthread_join(threads[i], NULL);
        total_dans_cercle += donnees[i].nb_points_dans_cercle;
    }
	clock_t tmp_end = clock();
	double tmp_cpu = (double)(tmp_end - tmp_beg) / CLOCKS_PER_SEC;
	printf ("temps d'exec (clock): %.6f sec\n", tmp_cpu);

    double proportion = (double)total_dans_cercle / (double)nb_points_total;
    double pi_estime = 4.0 * proportion;
	// Affichage
    printf("Nombre total de points : %ld\n", nb_points_total);
    printf("Nombre de threads      : %d\n", nb_threads);
    printf("Points dans le cercle  : %ld\n", total_dans_cercle);
    printf("Approximation de pi    : %.10f\n", pi_estime);

    free(threads);
    free(donnees);
    return EXIT_SUCCESS;
}