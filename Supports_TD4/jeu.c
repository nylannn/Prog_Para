#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define BS 3000 // BS = board size

// This function prints the given board on the console (without the border used to facilitate computation)
void print_board(int **board, int loop)
{
    printf("loop %d\n", loop);
    for (int i=1; i<= BS; i++) {
	for (int j=1; j<= BS; j++) {
	    if ( board[i][j] == 1)
		printf("X");
	    else
		printf(" ");
	}
	printf("\n");
    }
}

void print_ngb(int **ngb)
{
    printf("ngb :\n");
    for (int i=1; i<= BS; i++) {
	for (int j=1; j<= BS; j++) {
	    if ( ngb[i][j] > 1)
		printf("%d", ngb[i][j]);
	    else
		printf(" ");
	}
	printf("\n");
    }
}

/**
 * This function generates the initial board with one row and one
 * column of living cells in the middle of the board
 */
int generate_initial_board(int N, int **board)
{
    int num_alive = 0;

    for (int i = 1; i <= BS; i++) {
	for (int j = 1; j <= BS; j++) {
	    if (i == BS/2 || j == BS/2) {
		board[i][j] = 1;
		num_alive ++;
	    }
	    else {
		board[i][j] = 0;
	    }
	}
    }

    return num_alive;
}

int main(int argc, char* argv[])
{
    int num_alive, maxloop;
    int ldboard, ldnbngb;
 
    if (argc < 2) {
	maxloop = 10;
    } else {
	maxloop = atoi(argv[1]);
    }
    num_alive = 0;

    /* Leading dimension of the board array. We add a around the array to facilitate the computation */
    ldboard = BS + 2;
    /* Leading dimension of the neigbour counters array */
    ldnbngb = BS + 2;

    int** board = malloc( ldboard * sizeof(int*) );
    for(int i = 0; i < ldboard; i++)
	board[i] = malloc( ldboard * sizeof(int) );

    int **ngb = malloc( ldnbngb * sizeof(int*) );
    for(int i = 0; i < ldnbngb; i++)
	ngb[i] = malloc( ldnbngb * sizeof(int) );


    num_alive = generate_initial_board( BS, board);

    //print_board(board, 0 );

    printf("Starting number of living cells = %d\n", num_alive);

    for (int loop = 1; loop <= maxloop; loop++) {

	//Initialization of cells on the borders to help when computing the number of alived neighbors
	board[	 0][   0] = board[BS][BS];
	board[	 0][BS+1] = board[BS][ 1];
	board[BS+1][   0] = board[ 1][BS];
	board[BS+1][BS+1] = board[ 1][ 1];

	for (int i = 1; i <= BS; i++) {
	    board[   i][    0] = board[ i][ BS];
	    board[   i][ BS+1] = board[ i][  1];
	    board[   0][    i] = board[BS][  i];
	    board[BS+1][    i] = board[ 1][  i];
	}

	// This loop computes the number of alived neighbors of each cell
	for (int i = 1; i <= BS; i++) {
	    for (int j = 1; j <= BS; j++) {
		ngb[i][j] =
		    board[ i-1][ j-1 ] + board[ i][ j-1 ] + board[ i+1][ j-1 ] +
		    board[ i-1][ j   ] +                    board[ i+1][ j   ] +
		    board[ i-1][ j+1 ] + board[ i][ j+1 ] + board[ i+1][ j+1 ];
	    }
	}
	//print_ngb(ngb );
	// This loop updates the celles'state
	num_alive = 0;
	for (int i = 1; i <= BS; i++) {
	    for (int j = 1; j <= BS; j++) {
		if ( (ngb[i][j] < 2) || (ngb[i][j] > 3) ) 
		{
		    board[i][j] = 0;
		}
		else {
		    if (ngb[i][j] == 3)
			board[i][j] = 1;
		}
		if (board[i][j] == 1) {
		    num_alive ++;
		}
	    }
	}
        
	//print_board( board, loop);
	printf("Number of living cells after iteration %d = %d \n", loop, num_alive);

    }
    
    printf("Final number of living cells = %d\n", num_alive);

   
    // free memory
    for(int i = 0; i < ldboard; i++)
	free(board[i]);

    for(int i = 0; i < ldnbngb; i++)
	free(ngb[i]);

    free(board);
    free(ngb);

    return EXIT_SUCCESS;
}

