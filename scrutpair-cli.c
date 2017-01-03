#include "bor-util.h"                                                                                                                            

int boucle = 1;

void capter_fin(int sig)
{
    printf("signal %d capté\n", sig);
    boucle = 0;
}

int main(int argc, const char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "2 arguments requis: addresse du client et du serveur\n");
        exit(1);
    }
    const char* path = argv[1];
    const char* path_serv = argv[2];
    struct sockaddr_un addr;
    struct sockaddr_un addr_serv;
    int soc = bor_create_socket_un(SOCK_STREAM, path, &addr);
    if (soc < 0) {
        exit(1);
    }
    bor_set_sockaddr_un(path_serv, &addr_serv);
    printf("Connexion au serveur...\n");
    int co = bor_connect_un(soc, &addr_serv);
    if (co < 0) {
        goto fin1;
    }
    bor_signal(SIGPIPE, SIG_IGN, SA_RESTART);
    bor_signal(SIGINT, capter_fin, 0);
    int k = 0;
    while (boucle) {
        char buf[1000];
        fd_set set1;
        
        FD_ZERO(&set1);
        FD_SET(0, &set1);
        FD_SET(soc, &set1);

        k = select(soc+1, &set1, NULL, NULL, NULL);
        if (k < 0) {
            if (errno == EINTR) {
                printf("Signal recu\n");
            }
            else {
                perror("select");
            }
            goto fin1;
        }
        else {
            if (FD_ISSET(0, &set1)) {
                k = bor_read_str(0, buf, sizeof(buf));
                if (k <= 0) {
                    goto fin1;
                }
                printf("Envoi au serveur...\n");
                k = bor_write_str(soc, buf);
                if (k < 0) {
                    goto fin1;
                }
                printf("Attente de réponse...\n");
            }
            if (FD_ISSET(soc, &set1)) {
                k = bor_read_str(soc, buf, sizeof(buf));
                if (k <= 0) {
                    goto fin1;
                }
                printf("Réponse (%d octets): %s\n", k, buf);
                printf("Enter digits : \n");
            }
        }
    }
fin1:
    printf("Fin client\n");
    close(soc);
    unlink(addr.sun_path);
    return 0;
}

