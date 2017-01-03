#include "bor-util.h"                                                                                                                            

#define SIZE 3 
int boucle = 1;
int sockets[SIZE];

void preparer_reponse(char *buf1, char *buf2)   
{
    int i, j = 0;
    for (i = 0; buf1[i]; i++) {
        if (buf1[i] >= '0' && buf1[i] <= '9' && (buf1[i] - '0') % 2 == 0) {
            buf2[j++] = buf1[i];
        }
    }
    if (j == 0) {
        buf2[j++] = '-';
    }
    buf2[j] = 0;
}

int dialoguer_avec_client(int soc)
{
    char buf1[1000], buf2[1000];
    int k = bor_read_str(soc, buf1, sizeof(buf1));
    if (k <= 0) {
        return k;
    }
    printf("Serveur a lu : %s\n", buf1);
    preparer_reponse(buf1, buf2);
    printf("Serveur a repondu : %s\n", buf2);
    k = bor_write_str(soc, buf2);
    return k;
}

int cur_max_soc()
{
    int max = 0;
    int i;
    for (i = 0; i < SIZE; i++) {
        if (sockets[i] > max) {
            max = sockets[i];
        }
    }
    return max;
}

int traiter_client(int soc)
{
    fd_set set1;
    FD_ZERO(&set1);
    FD_SET(soc, &set1);
    int i;
    for (i = 0; i < SIZE; i++) {
        if (sockets[i] != 0) {
            FD_SET(sockets[i], &set1);
        }
    }
    int max = cur_max_soc();
    max = soc > max ? soc : max; 
    int r = select(max+1, &set1, NULL, NULL, NULL);
    if (r < 0) {
        if (errno == EINTR) {
            printf("Signal recu\n");
        }
        else {
            perror("select");
        }
        return r;
    }
    if (FD_ISSET(soc, &set1)) {
        printf("Nouvelle connexion !\n");
        struct sockaddr_un client;
        int soc_service = bor_accept_un(soc, &client);
        if (soc_service < 0) {
            return soc_service;
        }
        int i;
        for (i = 0; i < SIZE; i++) {
            if (sockets[i] == 0) {
                sockets[i] = soc_service; 
                break;
            }
            if (i == SIZE - 1) {
                int res = bor_write_str(soc_service, "Trop de clients connectés simultanement!\n");
                if (res < 0) {
                    return res;
                }
                close(soc_service);
                return 0;
            }
        }
        printf("Client connecté : %s\n", client.sun_path);
    }
    else {
        int i;
        for (i = 0; i < SIZE; i++) {
            if(sockets[i] != 0 && FD_ISSET(sockets[i], &set1)) {
                int res = dialoguer_avec_client(sockets[i]); 
                if (res <= 0) {
                    close(sockets[i]);
                    sockets[i] = 0;
                }
            }
        }
    }
    return 1;
}

void capter_fin(int sig)
{
    printf("signal %d capté\n", sig);
    boucle = 0;
}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "1 argument requis: addresse du serveur\n");
        exit(1);
    }
    const char* path = argv[1];
    struct sockaddr_un addr;
    int soc = bor_create_socket_un(SOCK_STREAM, path, &addr);
    if (soc < 0) {
        exit(1);
    }
    printf("Attente de clients...\n");
    int co = bor_listen(soc, 1);
    if (co < 0) {
        goto fin1;
    }
    bor_signal(SIGPIPE, SIG_IGN, SA_RESTART);
    bor_signal(SIGINT, capter_fin, 0);
    while (boucle) {
        int k = traiter_client(soc);
        if (k <= 0) {
            goto fin1;
        }
    }
fin1:
    printf("Fin serveur\n");
    close(soc);
    unlink(addr.sun_path);
    return 0;
}

