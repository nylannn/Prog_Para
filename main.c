#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

/* ************************************************************************************************
 * Exercice 1
 * ************************************************************************************************
 * Question 1 : Ecrivez un programme ayant le comportement suivant :
 * - Des threads sont créés (leur nombre est passé en paramètre lors du lancement du programme) ;
 * - Chaque thread affiche son PID et son identifiant ;
 * - Le thread principal (main) attend la terminaison des différents threads créés.
 * *********************************************************************************************** */

// Fonction executé par chaque thread
void * fonction_thread (void * arg)
{
                                    //getpid() retourne le PID du processus (le PID est partagé par tous les threads)
    pthread_t tid = pthread_self(); //pthread_self() permet de retourner l'identifiant unique d'un thread
    
    printf ("Thread PID : %d, Identifiant du thread : %p\n", getpid(), (void*)tid);
    
    return NULL;
}

int main(int argc, char **argv)
{
    // vérification systématique des arguments d'entrées : le nombre de thread doit etre passé en paramètre en amont
    if (argc != 2)
    {
        fprintf(stderr, "Usage : %s <nombre_de_threads>\n", argv[0]);
    }
    
    // Processus de creation d'un thread
    
        // Conversion du parametre
    int nb_threads = atoi(argv[1]); // atoi(...) et atoll(...) servent à retourner une chaine de caractère en int ou long long
        // Allocation des tableaux
    pthread_t * threads = malloc (nb_threads * sizeof(pthread_t));
    if (threads == NULL)
    {
        perror ("malloc");
        return EXIT_FAILURE;
    }
    
    printf ("création de %d threads\n", nb_threads);
    for (int i = 0; i < nb_threads; i++)
    {
        int err = pthread_create (&threads[i], NULL, fonction_thread, NULL);
        if (err != 0)
        {
            fprintf (stderr, "Erreur pthread_create : %d\n", err);
            free(threads);
            return EXIT_FAILURE;
        }
    }
    
    // Attente de la terminaison des threads
    printf ("Attente de la terminaison des threads : \n");
    for (int i = 0; i < nb_threads; i++)
    {
        int err = pthread_join(threads[i], NULL);
        if (err != 0)
        {
            fprintf (stderr, "Erreur pthread_join : %d \n", err);
        }
    }
    
    // Libération memoire
    free (threads);
    printf ("Tous les threads ont termine\n");
    return EXIT_SUCCESS;
    
    
    
    
	printf("hello world\n");
	return 0;
}
