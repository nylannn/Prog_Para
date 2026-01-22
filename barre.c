/*
 * Programme de barres de progression avec limitation à 2 barres simultanées
 * 
 * Contrainte: Au maximum 2 barres peuvent se remplir en même temps
 * Mécanisme: Sémaphore de comptage initialisé à 2
 * 
 * Compilation: gcc barres.c -o barres -L. -lbarx -lX11 -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <X11/Xlib.h>
#include <semaphore.h>   // ⭐ NOUVEAU: Pour le sémaphore
#include "barx.h"

#define nth 6      /* nombre de threads à lancer */
#define lng 40

char buf[lng];

/*
 * MÉCANISME DE SYNCHRONISATION [file:2]
 * 
 * Sémaphore de comptage pour limiter le nombre de barres actives
 * - Initialisé à 2 → permet à 2 threads de "progresser" simultanément
 * - sem_wait() décrémente (bloque si valeur = 0)
 * - sem_post() incrémente (libère un thread en attente)
 */
sem_t sem_barres;  // Sémaphore pour contrôler l'accès aux barres

/* Structure pour passer les paramètres */
typedef struct data {
    int numero;
    int mi;
} data;

char* colors[] = {"yellow", "black", "green", "blue", "pink", "white"};

/**
 * Fonction exécutée dans chaque thread
 * 
 * FONCTIONNEMENT:
 * 1. Le thread attend d'obtenir l'autorisation (sem_wait)
 * 2. Il remplit sa barre complètement
 * 3. Il libère l'autorisation (sem_post) pour un autre thread
 */
void *th_fonc(void *arg)
{
    data* d = (data*)arg;
    int numero = d->numero;
    int mi = d->mi;
    
    // Calcul des positions (barres paires à gauche, impaires à droite)
    int start_x1 = 30;
    int start_x2 = 100;
    int offset = numero;
    
    if (numero % 2 != 0) { 
        start_x1 = 350; 
        start_x2 = 400; 
        offset = numero - 1;
    }
    
    char str[16];
    snprintf(str, 16, "_%d_", numero);
    
    printf("Thread %d: créé, en attente d'autorisation...\n", numero);
    
    /*
     * ⭐ POINT CLÉ: DEMANDE D'AUTORISATION [file:2]
     * 
     * sem_wait(&sem_barres) effectue:
     * - Si sémaphore > 0: décrémente et continue (acquiert l'autorisation)
     * - Si sémaphore = 0: bloque le thread en attente passive
     * 
     * Comme le sémaphore est initialisé à 2:
     * - Les 2 premiers threads passent immédiatement
     * - Les threads suivants attendent qu'un des 2 premiers libère
     */
    sem_wait(&sem_barres);
    
    printf("Thread %d: AUTORISATION OBTENUE, début du remplissage\n", numero);
    
    // Affichage du numéro et création du rectangle vide
    drawstr(start_x1, 50 + 35*offset, str, 3);
    drawrec(start_x2, 25 + offset*35, 25 + mi*10, 30);
    flushdis();
    
    /*
     * REMPLISSAGE COMPLET DE LA BARRE
     * Une fois l'autorisation obtenue, le thread remplit sa barre en entier
     */
    for (int j = 1; j <= mi; j++)
    {
        // Délai variable selon le thread pour visualiser l'alternance
        usleep(500000 + 100000*numero);
        
        // Remplissage progressif
        fillrec(start_x2, 27 + offset*35, 27 + j*10, 26, colors[numero]);
        flushdis();
    }
    
    printf("Thread %d: remplissage TERMINÉ, libération de l'autorisation\n", numero);
    
    /*
     * ⭐ POINT CLÉ: LIBÉRATION DE L'AUTORISATION [file:2]
     * 
     * sem_post(&sem_barres) effectue:
     * - Incrémente le sémaphore
     * - Si des threads sont en attente, en réveille un
     * 
     * Cela permet à un thread en attente de commencer son remplissage
     */
    sem_post(&sem_barres);
    
    return NULL;
}

/* Fonction pour mettre le programme en pause */
int liretty(char *prompt, char *buffer)
{
    int i;
    printf("\n%s", prompt);
    i = scanf("%s", buffer);
    return strlen(buffer);
}

int main(int argc, char** argv)
{
    int nlu, i;
    
    // Initialisation thread-safe de X11
    XInitThreads();
    
    // Création de la fenêtre graphique
    initrec();
    
    printf("=== Programme de barres de progression ===\n");
    printf("Contrainte: maximum 2 barres se remplissent simultanément\n\n");
    
    /*
     * ⭐ INITIALISATION DU SÉMAPHORE [file:2]
     * 
     * sem_init(&sem_barres, 0, 2) signifie:
     * - &sem_barres: adresse du sémaphore
     * - 0: partagé uniquement entre threads (pas entre processus)
     * - 2: VALEUR INITIALE = 2 autorisations disponibles
     * 
     * C'est un SÉMAPHORE DE COMPTAGE (pas binaire)
     * Il permet de limiter à 2 le nombre de threads dans leur section critique
     */
    sem_init(&sem_barres, 0, 2);
    
    // Tableau de threads
    pthread_t threads[nth];
    
    // Allocation et initialisation des paramètres
    data* tab = malloc(sizeof(data) * nth);
    
    // Longueurs différentes pour visualiser l'alternance
    int longueurs[nth] = {20, 25, 15, 30, 18, 22};
    
    // Création des threads
    printf("=== Création de %d threads ===\n\n", nth);
    for (i = 0; i < nth; i++)
    {
        tab[i].numero = i;
        tab[i].mi = longueurs[i];
        
        int err = pthread_create(&threads[i], NULL, th_fonc, &tab[i]);
        if (err != 0) {
            fprintf(stderr, "Erreur pthread_create thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    
    // Attente de la terminaison de tous les threads
    printf("\n=== Attente de la terminaison ===\n");
    for (i = 0; i < nth; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n=== Tous les threads ont terminé ===\n");
    
    /*
     * DESTRUCTION DU SÉMAPHORE [file:2]
     * Libère les ressources associées
     */
    sem_destroy(&sem_barres);
    
    // Libération mémoire
    free(tab);
    
    // Pause avant fermeture
    nlu = liretty("Appuyez sur Entrée pour sortir", buf);
    
    // Destruction de la fenêtre
    detruitrec();
    
    return EXIT_SUCCESS;
}
 /*
 Réponse à la question - Exercice 2
>> Stratégie de synchronisation
J'ai utilisé un SÉMAPHORE DE COMPTAGE pour limiter à 2 le nombre de barres se remplissant simultanément.
​

Principe de fonctionnement :

Initialisation : sem_init(&sem_barres, 0, 2)

Le sémaphore est initialisé avec la valeur 2

Cela représente 2 "autorisations" disponibles

Demande d'autorisation : sem_wait(&sem_barres) avant le remplissage

Si le sémaphore > 0 : décrémente et le thread continue

Si le sémaphore = 0 : le thread est bloqué en attente passive

Les 2 premiers threads obtiennent l'autorisation immédiatement

Les threads suivants attendent

Libération d'autorisation : sem_post(&sem_barres) après le remplissage complet

Incrémente le sémaphore (libère une autorisation)

Réveille un thread en attente s'il y en a

Permet à une nouvelle barre de commencer

Pourquoi un sémaphore et pas un mutex ?
​

Mutex : Binaire (0 ou 1), limite à 1 seul thread

Sémaphore de comptage : Peut avoir n'importe quelle valeur, permet de limiter à N threads simultanés

Ici N=2, donc sémaphore initialisé à 2

Déroulement typique :

Threads 0 et 5 démarrent (sémaphore passe de 2 → 0)

Threads 1,2,3,4 sont bloqués en attente

Thread 0 termine → sem_post() → sémaphore = 1 → Thread 4 démarre

Thread 5 termine → sem_post() → sémaphore = 2 → Thread 2 démarre

Et ainsi de suite...

Avantages :

Simple à implémenter (2 lignes de code : sem_wait + sem_post)

Équitable : ordre d'exécution dépend de l'ordonnanceur

Pas de busy-wait : threads en attente passive (efficace)
*/