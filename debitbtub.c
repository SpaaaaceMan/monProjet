#include "bor-util.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

size_t res;

void alarmHandler (int sig) {
    printf("nb carac lu : %zu ko/s\n", res / 1000);
    res = 0;
    alarm(1);
}

int monPipe (int p[]) {
    int res = pipe (p);
    if (res < 0) {
        perror ("pipe");
        exit (1);
    }
    return res;
}

int monMax2(int x, int y) {
    if (x > y) return x;
    return y;
}

pid_t monFork () {
    int f = fork ();
    if (f < 0) {
        perror ("fork");
        exit (1);
    }
    return f;
}

int monMax (int** tab, int size) {
    int max = 0;
    printf ("test\n");

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 2; ++j) {
            printf ("Fin\n");

            if (tab[i][j] > max) max = tab[i][j];
        }
    }
    return max;
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Two arguments expected !\n");
        exit (1);
    }

    size_t sizeBuf = (size_t) strtol (argv[1], (char **)NULL, 10);
    int nbPipes = (int) strtol (argv[2], (char **)NULL, 10);

    char s[sizeBuf];
    int pipes[nbPipes][2];

    for (int i = 0; i < nbPipes; ++i) {
        monPipe(pipes[i]);
    }

    pid_t f = monFork ();

    //fils
    if (f == 0) {
        bor_signal(SIGALRM, alarmHandler, SA_RESTART);

        for (int i = 0; i < nbPipes; ++i) {
            close(pipes[i][1]);
        }

        fd_set fd_set1;

        alarm (1);

        while (1) {
            //reset la liste
            FD_ZERO (&fd_set1);

            //ajout tubes
            for (int i = 0; i < nbPipes; ++i) {
                FD_SET (pipes[i][0], &fd_set1);
            }
            int max = 0;
            for (int i = 0; i < nbPipes; ++i) {
                max = monMax2(pipes[i][0], max);
            }

            int selectResult = select (max + 1, &fd_set1, NULL, NULL, NULL);
            if (selectResult < 0) {
                if (selectResult == EINTR) {
                    printf("Signal reçu\n");
                }
                else {
                    perror ("select");
                    exit (1);
                }
            }
            else if (selectResult == 0) {
                printf("Time Out\n");
                break;
            }
            for (int i = 0; i < nbPipes; ++i) {
                if (FD_ISSET (pipes[i][0], &fd_set1)) {
                    res += bor_read (pipes[i][0], &s, sizeBuf);
                    if (res == 0) break;
                }
            }
        }
        exit (0);
    }

    // Dans le père
    for (int i = 0; i < nbPipes; ++i) {
        close(pipes[i][0]);
    }

    fd_set fd_set1;

    while (1) {

        //reset la liste
        FD_ZERO (&fd_set1);

        //ajout tubes
        for (int i = 0; i < nbPipes; ++i) {
            FD_SET (pipes[i][1], &fd_set1);
        }

        int max = 0;
        for (int i = 0; i < nbPipes; ++i) {
            max = monMax2(pipes[i][1], max);
        }

        int selectResult = select (max + 1, NULL, &fd_set1, NULL, NULL);
        if (selectResult < 0) {
            if (selectResult == EINTR) {
                printf("Signal reçu\n");
            }
            else {
                perror ("select");
                exit (1);
            }
        }
        else if (selectResult == 0)
        {
            printf("Time Out\n");
            break;
        }
        for (int i = 0; i < nbPipes; ++i) {
            if (FD_ISSET (pipes[i][1], &fd_set1)) {
                bor_write (pipes[i][1], &s, sizeBuf);
            }
        }
    }
    return 0;
}
