#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"


void* printColoredSquare(void* arg) {
    const char* color = (const char*)arg;
    
    while (1) {
        if (color == ANSI_COLOR_BLUE) {
            printf("%s■%s ", color, ANSI_COLOR_RESET);
	    fflush(stdout);
	    usleep(150000);
        } else if (color == ANSI_COLOR_WHITE) {
            printf("%s■%s ", color, ANSI_COLOR_RESET);
	    fflush(stdout);
	    usleep(150000);
        } else if (color == ANSI_COLOR_RED) {
            printf("%s■%s ", color, ANSI_COLOR_RESET);
	    fflush(stdout);
	    usleep(150000);
        }
	
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
       
    return 0;
}

