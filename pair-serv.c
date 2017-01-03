#include "bor-util.h"                                                                                                                            

int boucle = 1;
int boucle_client = 1;

void preparer_reponse(char *buf1, char *buf2)   
{
    int i, j = 0;
    for (i = 0; buf1[i]; i++) {
        if (buf1[i] >= '0' && buf1[i] <= '9' && (buf1[i] - '0')% 2 == 0) {
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
    printf("Serveur fils a lu...\n");
    preparer_reponse(buf1, buf2);
    k = bor_write_str(soc, buf2);
    return k;
}

void main_fils(int soc)
{
    int k = -1;
    while (boucle_client) {
        k = dialoguer_avec_client(soc);
        if (k <= 0) {
            break;
        }
    }
    printf("Fin client\n");
    close(soc);
    exit(k < 0 ? 1 : 0);
}

int traiter_client(int soc)
{
    int boucle_client = 1;
    char buf[1000];
    struct sockaddr_un client;
    int soc_service = bor_accept_un(soc, &client);
    if (soc_service < 0) {
        return soc_service;
    }
    printf("Client connecté : %s\n", client.sun_path);
    int f = fork();
    if (f < 0) {
        perror("fork");
        close(soc_service);
        return f;
    }
    if (f == 0) {
        close(soc);
        main_fils(soc_service);
        exit(0);
    }
    close(soc_service);
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

