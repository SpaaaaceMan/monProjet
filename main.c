#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
/*
int main(void)
{
    int pipefd[2];
    int p = pipe(pipefd);

    //si échec
    if (p < 0) {
        perror("pipe");
        exit (1);
    }
    pid_t fils = fork();

    //si échec
    if (fils < 0) {
        perror("fork");
        exit (1);
    }

    //si dans le fils
    if (fils == 0) {
        int cpt = 10;
        close (pipefd[0]);
        while (cpt > 0) {
            char buf[2] = {'a', '\0'};
            if (write (pipefd[1], buf, 2) < 0) {
                perror ("write");
                exit(1);
            }
            --cpt;
            sleep (1);
        }
        // fin du fils
        exit (0);
    }

    // Dans le père
    close(pipefd[1]);
    fd_set fd_set1;
    int cpt = 0;

    while (1) {

        //reset la liste
        FD_ZERO (&fd_set1);

        //ajout tube
        FD_SET (pipefd[0], &fd_set1);

        //ajout entrée standard
        FD_SET (0, &fd_set1);
        int s = select (pipefd[0] + 1, &fd_set1, NULL, NULL, NULL);
        if (s < 0) {
            if (s == EINTR) {
                printf("Signal reçu\n");
            }
            else {
                perror ("select");
                exit (1);
            }
        }
        else if (s == 0)
            printf("Time Out\n");

        if (FD_ISSET (0, &fd_set1)) {
            char buf[1000];
            int r = read (0, buf, 1000);
            if (r < 0) {
                perror ("read");
                exit (1);
            }
            if (r == 0) exit (0);
            printf("Lu %d carac sur fd = %d :\"%s\"\n", r, 0, buf);
        }
        if (FD_ISSET (pipefd[0], &fd_set1)) {
            char buf[1000];
            int r = read (pipefd[0], buf, 1000);
            if (r < 0) {
                perror ("read");
                exit (1);
            }
            if (r == 0) exit (0);
            cpt++;
            printf("%dLu %d carac sur fd = %d :\"%s\"\n", cpt, r, pipefd[0], buf);
        }
    }

    printf("Hello World!\n");
    return 0;
}
*/
