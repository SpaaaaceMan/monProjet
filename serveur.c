#include "bor-util.h"

#define SLOTS_NB 3

typedef enum {E_LIBRE, E_LIRE_REQUETE, E_ECRIRE_REQUETE} Etat;

typedef struct  {
    Etat etat;
    int soc;
    struct sockaddr_in adr;
} Slot;

typedef struct {
    Slot slots[SLOTS_NB];
    int soc_ec;
    struct sockaddr_in adr;
} Serveur;

void init_slot(Slot *o)
{
    o->etat = E_LIBRE;
    o->soc = -1;
    memset(&o->adr, 0, sizeof(o->adr));
}

int slot_est_libre(Slot *o)
{
    return o->etat == E_LIBRE;
}

void liberer_slot(Slot *o)
{
    if (slot_est_libre(o)) return;
    init_slot(o);
}

void init_serveur(Serveur *s)
{
    printf("Serveur : Initialisation ! \n");
    int i;
    for (i = 0; i < SLOTS_NB; i++) {
        Slot slot;
        s->slots[i] = slot;
        init_slot(&s->slots[i]);
    }
    s->soc_ec = -1;
    printf("Serveur : Initialisé ! \n");
}

int chercher_slot_libre(Serveur *s)
{
    int i; 
    for (i = 0; i < SLOTS_NB; i++) {
        if (slot_est_libre(&s->slots[i])) break;
    }
    if (i == SLOTS_NB) {
        i = -1;
    }
    return i;
}

int demarrer_serveur(Serveur *s, int port)
{
    printf("Serveur : Démarrage ! \n");
    init_serveur(s);
    int soc = bor_create_socket_in(SOCK_STREAM, port, &s->adr);
    if (soc < 0) {
        return -1;
    }
    int ec = bor_listen(soc, 1);
    if (ec < 0) {
        return -1;
    }
    s->soc_ec = soc;
    printf("Serveur : Démarré ! \n");
    return 0;
}

void fermer_serveur(Serveur *s)
{
    printf("Serveur : Fermeture ! \n");
    close(s->soc_ec);
    int i;
    for (i = 0; i < SLOTS_NB; i++) {
        liberer_slot(&s->slots[i]);
    }
    printf("Serveur : Fermé ! \n");
}

int accepter_connexion(Serveur *s)
{
    printf("Serveur : Connexion en cours... \n");
    struct sockaddr_in client;
    int k = bor_accept_in(s->soc_ec, &client);
    int slotLibre = chercher_slot_libre(s);
    if (slotLibre < 0) {
        printf("Serveur : Aucun slots libre ! \n");
        close(k);
        return slotLibre;
    }
    Slot *slot = &s->slots[slotLibre];
    slot->etat = E_LIRE_REQUETE;
    slot->adr = client;
    slot->soc = k;
    printf("Serveur[%d] : Client %s Connecté !\n", slotLibre, bor_adrtoa_in(&client));
    return 0;
}

int proceder_lecture_requete(Slot *o)
{
    char buf[1000];
    int k = bor_read_str(o->soc, buf, sizeof(buf));
    printf("%d\n", k);
    if (k < 0) {
        printf("%d\n", k);
        return 0;
    }
    o->etat = E_ECRIRE_REQUETE;
    return 1;
}

int proceder_ecrire_reponse(Slot *o)
{
    int k = bor_write_str(o->soc, "Serveur en construction, mais ça marche plutôt bien !");
    printf("%d\n", k);
    if (k < 0) {
        printf("%d\n", k);
        return 0;
    }
    o->etat = E_LIRE_REQUETE;
    return 1;
}

void traiter_slot_si_eligible(Slot *o, fd_set *set_read, fd_set *set_write)
{
    if (slot_est_libre(o)) return;
    int res = 1;
    if (o->etat == E_LIRE_REQUETE) {
        if (FD_ISSET(o->soc, set_read)) {
            res = proceder_lecture_requete(o);
        }
    }
    else if (o->etat == E_ECRIRE_REQUETE) {
        if (FD_ISSET(o->soc, set_write)) {
            res = proceder_ecrire_reponse(o);
        }
    }

    if (res <= 0) {
        liberer_slot(o);
    }
}

void preparer_select(Serveur *s, int *maxfd, fd_set *set_read, fd_set *set_write)
{
    FD_ZERO(set_read);
    FD_ZERO(set_write);
    int i;
    *maxfd = 0;
    for (i = 0; i < SLOTS_NB; i++) {
        if (s->slots[i].soc > *maxfd) {
            *maxfd = s->slots[i].soc;
        }
        if (s->slots[i].etat == E_LIRE_REQUETE) {
            FD_SET(s->slots[i].soc, set_read);
        }
        else if (s->slots[i].etat == E_ECRIRE_REQUETE) {
            FD_SET(s->slots[i].soc, set_write);
        }
    }
    FD_SET(s->soc_ec, set_read);
    *maxfd = s->soc_ec > *maxfd ? s->soc_ec : *maxfd;

}

int faire_scrutation(Serveur *s)
{
    fd_set set_read;
    fd_set set_write;
    int maxfd;

    preparer_select(s, &maxfd, &set_read, &set_write);

    //printf("%d\n", maxfd);

    int k = select(maxfd + 1, &set_read, &set_write, NULL, NULL);


    if (k < 0) {
        return -1;
    }
    
    if(FD_ISSET(s->soc_ec, &set_read)) {
        accepter_connexion(s);
    }
    size_t i;
    for (i = 0; i < SLOTS_NB; i++) {
        traiter_slot_si_eligible(&s->slots[i], &set_read, &set_write);
    }
    
    return 1;
}

int boucle = 1;

void capter_fin(int sig)
{
    printf("signal %d capté\n", sig);
    boucle = 0;
}

int main(int argc, const char *argv[])
{
    if (argc < 1){
        printf("too few arguments");
        return 1;
    }
    Serveur s;
    int port = atoi(argv[1]);
    demarrer_serveur(&s, port);
    bor_signal(SIGPIPE, SIG_IGN, SA_RESTART);
    bor_signal(SIGINT, capter_fin, 0);
    
    while(boucle)
        faire_scrutation(&s);
    
    fermer_serveur(&s);
    return 0;
}
