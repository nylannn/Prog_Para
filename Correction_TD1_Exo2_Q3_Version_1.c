#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
	int ordre;
	pthread_mutex_t mutex;
	unsigned long somme;
} data;

void* task(void* a)
{
	data* d = (data*) a;
	pthread_mutex_lock(&(d->mutex));
	int myordre = d->ordre;
	d->ordre +=1;
	pthread_mutex_unlock(&(d->mutex));


    	printf("Mon id = %p, Mon PID = %d, Mon ordre = %d\n", (void*)pthread_self(), getpid(), myordre); 
	int i;

	//pthread_mutex_lock(&(d->mutex));
	for(i = 0; i < 100000; i++)
	{
		pthread_mutex_lock(&(d->mutex));
		d->somme += myordre;
		pthread_mutex_unlock(&(d->mutex));
	}
	//pthread_mutex_unlock(&(d->mutex));
    	return NULL;
}

int main(int argc, char** argv)
{
    	int i, res, nb;
	printf("argc = %d\n", argc);
	if(argc != 2)
	{
		perror("erreur nombre d'arguments");
		return 1;
	}
    	nb = atoi(argv[1]);
    	pthread_t *threads;
    	threads = malloc(nb * sizeof(pthread_t));

	data d;
	d.somme = 0;
	d.ordre = 0;
	pthread_mutex_init(&(d.mutex), NULL);
    	
    	for(i =0; i < nb; i++)
   	{	
        	res = pthread_create(&threads[i], NULL, task, &d);
        	if(res != 0) perror("erreur creation thread");
    	}
   
    	for(i =0; i < nb; i++)
	{
       	 	res = pthread_join(threads[i],NULL);
		if(res != 0) perror("erreur join thread");
	}

	printf("somme =  %lu\n", d.somme);
    	free(threads);
	return 0;
}
