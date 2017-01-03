#include "bor-util.h"

#define MAX_CLIENTS 50

typedef struct CLIENT {
    char* addresse;
    int cptMsgRecu;
} CLIENT;

CLIENT* clientsConnus[MAX_CLIENTS];
int lastIndex = 0;

int dialoguer_avec_un_client(int soc) 
{
    if (lastIndex == MAX_CLIENTS) {
        printf("Aucun nouveau client admis !\n");
        return -1;
    }
    struct sockaddr_un adr_client; 
    printf("Attente d'un message client\n");
    char buf[100];
    int k = bor_recvfrom_un_str(soc, buf, sizeof(buf), &adr_client);
    if (k < 0) {
        return -1;
    }
    int i;
    for (i = 0; i <= lastIndex; i++) {
        printf("test\n");
        if (i == lastIndex) {
            CLIENT currentClient;
            currentClient.cptMsgRecu = 0;
            currentClient.addresse = strdup(adr_client.sun_path);
            clientsConnus[lastIndex] = &currentClient;
            ++lastIndex;
            printf("Nouveau Client ajouté\n");
            break;
        }
        char* addr = clientsConnus[i]->addresse;
        if (strcmp(adr_client.sun_path, addr) == 0) {
            clientsConnus[i]->cptMsgRecu += 1;
            printf("Client reconnu\n");
            break;
        }
    }
    printf("Reçu %d octets de %s: \"%s\"\n", k, clientsConnus[i]->addresse, buf);

    //réponse
    if (strcmp(buf, "NUMBER") == 0){
        char rep[100];
        sprintf(rep, "%d\n", clientsConnus[i]->cptMsgRecu);
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
    while (boucle_princ) {
        k = dialoguer_avec_un_client(soc);
        if (k < 0) {
            break;
        }
    }  
    printf("Fin du serveur\n");
    close(soc);
    unlink(sa.sun_path);
    exit (k < 0 ? 1 : 0);
    return 0;
}
