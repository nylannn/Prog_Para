#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include "barx.h"

#define nth 3	/* nbre de threads a lancer */
#define ifer(is,mess) if (is==-1) perror(mess)

#define lng 40
char buf[lng];

/* ------------------------------------------------------------- */

typedef struct data{
	int numero;
	int mi;
	char* couleur;
} data;

/* routine exécutée dans les threads */
void *th_fonc (void * arg)
{
	data* d = (data*)arg;
  	int numero = d->numero;
	int mi = d->mi;
	char str[16];
	snprintf (str, 16, "_%d_", numero);
		
	printf("ici thread %d\n",numero);	

	drawstr (30, 125+35*numero, str, 3);
	drawrec (100,100+numero*35,100+mi*10,30);
	flushdis ();

	
	for (int j=1;j<=mi;j++)
	{
		usleep(500000+100000*numero);
		fillrec (100,102+numero*35,100+j*10,26,d->couleur);
	}
	flushdis ();

	 
	//valeur de retour du thread 
	int* res = malloc(sizeof(int));
	*res= numero;
  	return (res );

}

int liretty (char *prompt, char *buffer)
{	int i;
	printf("\n%s",prompt);
	i = scanf ("%s",buffer);
	return strlen(buffer);
}


int main (int argc, char** argv)
{  
	int nlu, is, i,j;
	XInitThreads(); /* Pour ne pas utiliser le mutex et utiliser la librairie en mode thread-safe. */
	initrec();	/* creer rectangle rouge */
	

	pthread_t threads[nth];

	// Var couleurs
	char yellow[] = "yellow";
	char white[] = "white";
	char green[] = "green";

	//Arguments des threads  	
	data* tab = malloc(sizeof(data)*nth);
	tab[0].couleur = yellow;
	tab[1].couleur = white;
	tab[2].couleur = green;
	
	/* créer les threads */
  	for(i=0; i<nth; i++)
  	{ 
		tab[i].numero = i;
		tab[i].mi = 20+i*10;
		printf("ici main, création thread %d\n",i);
    		is = pthread_create( &threads[i], NULL, th_fonc, (void *)&tab[i]);
    		ifer (is,"err. création thread");
  	}


	/* attendre fin des threads */
 	int *val;
	for(i=0; i<nth; i++)
  	{ 
    		is = pthread_join(threads[i], (void**)&val);
    		ifer (is,"err. join thread");
    		printf("ici main, fin thread numero %d val=%d\n",i,*val);
		free(val);
  	}
	

	/* attendre une entrée utilisateur avant de fermer le programme */
	nlu = liretty("sortir ?",buf);
	printf("--fin--\n");
	detruitrec();	/* detruire la fenetre rectangle */
	exit(EXIT_SUCCESS);

}

