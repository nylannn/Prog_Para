#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include "barx.h"

#define N 10           /* taille du damier NxN */
#define CELL 15       /* taille d'une case en pixels */

char* colors[] = {"black", "green", "blue", "yellow"};

int liretty (char *prompt, char *buffer)
{
    int i;
    printf("\n%s", prompt);
    i = scanf("%s", buffer);
    return i;
}

int main(int argc, char** argv)
{
    char buf[32];

    XInitThreads();     
    initrec();          /* création de la fenêtre */

    int start_x = 30;
    int start_y = 30;

    /* dessin du damier */
    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            int x = start_x + j * CELL;
            int y = start_y + i * CELL;

            /* alternance des couleurs */
            int color_index = (i + j) % 4;

            /* dessiner le contour */
            drawrec(x, y, CELL, CELL);

            /* remplir la case */
            fillrec(x + 1, y + 1, CELL - 2, CELL - 2, colors[color_index]);
	    flushdis();
	    usleep(100000);

        }
    }    

    /* attendre entrée utilisateur */
    liretty("Appuyez sur une touche pour quitter", buf);

    detruitrec();       /* destruction de la fenêtre */
    return EXIT_SUCCESS;
}

