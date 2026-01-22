/******************** Fichier bar.c **********************/
/*********************************************************/
// Exemple d'utilisation des fonctions definies dans la librairie barx 
// Show you how to use the barx library functions
         
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "barx.h"
         
char buf[20];
/* ----------------------------------------------------- */
                  
void *b_fonc (void * arg)
{
	int is, numero,j, m1;
	numero = *(int*)arg;         
	m1 = 20; 

	
	printf("numero= %d, m1=%d \n",numero,m1);
	/* display the number and draw a rectangle*/
	drawstr (30, 125, "_0_", 3);
	drawrec (100,100,100+m1*10,30);
	
	for (j=1;j<=m1;j++)
	{	
		printf("num %d j=%d\n",numero,j);
		/* usleep is used to slow down the filling so that it is visible for us (too fast otherwise) */
		usleep(500000);
		fillrec (100,102,100+j*10,26,"yellow");
	}
	flushdis ();
	
	int *res = malloc(sizeof(int));
	*res = numero+100;
	return res;
}
 
/* Cette fonction est utilisée pour mettre le programme en "pause", i.e. en attente d'un saisie de l'utilisateur */
int liretty (char *prompt, char *buffer)
{	
	int i;
	printf("\n%s",prompt);
	i = scanf ("%s",buffer);
	return strlen(buffer);
}         
         
int main ()
{ 
	int nlu, *res, i=0;
         
	initrec();	/* creer une fenetre rectangle rouge - create a red window */
         
	res = b_fonc( &i );
	printf("is= %d\n",*res);
	free(res);
    
	
	nlu = liretty("sortir ?",buf);
	printf("--fin--\n");
         
	detruitrec();	/* detruire la fenetre rectangle - destroy window */
	exit(EXIT_SUCCESS);         
}
