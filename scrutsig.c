#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
/*
int monPipe (int p[]) {
    int res = pipe (p);
    if (res < 0) {
        perror ("pipe");
        exit (1);
    }
    return res;
}

pid_t monFork () {
    int f = fork ();
    if (f < 0) {
        perror ("fork");
        exit (1);
    }
    return f;
}

int main(void)
{
    //création des pipes
    int pipefd1[2];
    int pipefd2[2];
    monPipe (pipefd1);
    monPipe (pipefd2);

    //création du fils 1
    int f1 = monFork ();
    if (f1 == 0) {
        int cpt = 5;
        close (pipefd1[0]);
        while (cpt > 0) {
            char buf[2] = {'a', '\0'};
            if (write (pipefd1[1], buf, 2) < 0) {
                perror ("write");
                exit(1);
            }
            --cpt;
            sleep (4);
        }
        exit (0);
    }

    //création du fils 2
    int f2 = monFork ();
    if (f2 == 0) {
        int cpt = 5;
        close (pipefd2[0]);
        while (cpt > 0) {
            char buf[2] = {'b', '\0'};
            if (write (pipefd2[1], buf, 2) < 0) {
                perror ("write");
                exit(1);
            }
            --cpt;
            sleep (6);
        }
        exit (0);
    }

    //père
    close (pipefd1[1]);
    close (pipefd2[1]);
    fd_set fd_set1;
    struct timeval tv;

    int p1Valide = 1;
    int p2Valide = 1;

    while (1) {

        //reset la liste
        FD_ZERO (&fd_set1);

        //ajout tubes
        if (p1Valide)
            FD_SET (pipefd1[0], &fd_set1);
        if (p2Valide)
            FD_SET (pipefd2[0], &fd_set1);

        tv.tv_sec  = 3;
        tv.tv_usec = 0;

        int max = fmax(pipefd1[0], pipefd2[0]);
        int s = select (max + 1, &fd_set1, NULL, NULL, &tv);
        if (s < 0) {
            if (s == EINTR) {
                printf("Signal reçu\n");
            }
            else {
                perror ("select");
                exit (1);
            }
        }
        if (s == 0) {
            printf("Time out\n");
            if (p1Valide + p2Valide == 0) break;
            else continue;
        }

        if (FD_ISSET (pipefd1[0], &fd_set1)) {
            char buf[1000];
            int r = read (pipefd1[0], buf, 1000);
            if (r < 0) {
                perror ("read");
                exit (1);
            }
            if (r == 0) {
                p1Valide = 0;
                break;
            }
            printf("Lu %d carac sur fd = %d :\"%s\"\n", r, pipefd1[0], buf);
        }
        if (FD_ISSET (pipefd2[0], &fd_set1)) {
            char buf[1000];
            int r = read (pipefd2[0], buf, 1000);
            if (r < 0) {
                perror ("read");
                exit (1);
            }
            if (r == 0) {
                p2Valide = 0;
                break;
            }
            printf("Lu %d carac sur fd = %d :\"%s\"\n", r, pipefd2[0], buf);
        }
    }

    exit (0);

    printf("Hello World!\n");
    return 0;
}
*/

