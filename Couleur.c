#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_WHITE "\x1b[37m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

// Variables globales pour synchronisation
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int tour = 0;  // 0: bleu, 1: blanc, 2: rouge

void* printColoredSquare(void* arg) {
    const char* color = (const char*)arg;
    int mon_tour;
    if (color == ANSI_COLOR_BLUE) mon_tour = 0;
    else if (color == ANSI_COLOR_WHITE) mon_tour = 1;
    else mon_tour = 2;

    while (1) {
        pthread_mutex_lock(&mutex);
        // Attendre tant que ce n'est pas mon tour
        while (tour != mon_tour) {
            pthread_cond_wait(&cond, &mutex);
        }
        // Afficher
        printf("%s■%s ", color, ANSI_COLOR_RESET);
        fflush(stdout);
        usleep(150000);
        // Passer au tour suivant (cycle 0->1->2->0)
        tour = (tour + 1) % 3;
        pthread_cond_broadcast(&cond);  // Réveiller tous les threads
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char** argv) {
    pthread_t threadBlue, threadWhite, threadRed;
    pthread_create(&threadBlue, NULL, printColoredSquare, (void*)ANSI_COLOR_BLUE);
    pthread_create(&threadWhite, NULL, printColoredSquare, (void*)ANSI_COLOR_WHITE);
    pthread_create(&threadRed, NULL, printColoredSquare, (void*)ANSI_COLOR_RED);

    pthread_join(threadBlue, NULL);
    pthread_join(threadWhite, NULL);
    pthread_join(threadRed, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
