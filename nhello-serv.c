#include "bor-util.h"
int cptMsgRecu;

int dialoguer_avec_un_client(int soc) 
{
    struct sockaddr_un adr_client; 
    printf("Attente d'un message client\n");
    char buf[100];
    int k = bor_recvfrom_un_str(soc, buf, sizeof(buf), &adr_client);
    if (k < 0) {
        return -1;
    }
    printf("Reçu %d octets de %s: \"%s\"\n", k, adr_client.sun_path, buf);

    //réponse
    if (strcmp(buf, "NUMBER") == 0){
        char rep[100];
        sprintf(rep, "%d\n", cptMsgRecu);
        printf("Envoi de la réponse...\n");
        k = bor_sendto_un_str(soc, rep, &adr_client);
    }
    return k;
}

int boucle_princ = 1;

void capter_fin(int sig)
{
    boucle_princ = 0;
    printf("capte_fin %d\n", sig);
}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "One argument require\n");
    }
    const char* path = argv[1];
    struct sockaddr_un sa;
    int soc = bor_create_socket_un(SOCK_DGRAM, path, &sa);    
    if (soc < 0) {
        exit(1);
    }
    bor_signal(SIGINT, capter_fin, 0);
    int k;
    cptMsgRecu = 0;
    while (boucle_princ) {
        k = dialoguer_avec_un_client(soc);
        if (k < 0) {
            break;
        }
        else
            ++cptMsgRecu;
    }  
    printf("Fin du serveur\n");
    close(soc);
    unlink(sa.sun_path);
    exit (k < 0 ? 1 : 0);

    return 0;
}
