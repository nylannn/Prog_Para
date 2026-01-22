#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
	int ordre;
	pthread_mutex_t* mutex;
	int* somme;
} data;


void* task(void* a)
{
	data* d = (data*)a;
    	//printf("Je suis le thread %p et mon PID = \n", thread_self(), getpid()); 
	printf("Mon numero  = %d\n", d->ordre); 
	int i;
	for(i = 0; i < 100000; i++)
	{
		pthread_mutex_lock(d->mutex);
		*(d->somme)+=d->ordre;
		pthread_mutex_unlock(d->mutex);
	}
    return NULL;
}

int main(int argc, char** argv)
{
    printf("Debut main \n");
    int i, res, nb;
    nb = atoi(argv[1]);
    pthread_t *threads;
    threads = malloc(nb * sizeof(pthread_t));
		
	int somme =0;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	data* DB = malloc(nb*sizeof(data));
		

    for(i =0; i < nb; i++)
    {
		DB[i].somme = &somme;
		DB[i].mutex = &mutex;
		DB[i].ordre = i;
        res = pthread_create(&threads[i], NULL, task, &DB[i]);
        if(res != 0) perror("erreur creation thread");
    }
   
    for(i =0; i < nb; i++)
	{
        res = pthread_join(threads[i],NULL);
		if(res != 0) perror("erreur join thread");
	}

    printf("Fin main - somme = %d \n", somme);
    free(threads);
	free(DB);
    return 0;
}