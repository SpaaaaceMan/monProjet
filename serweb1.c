#include "bor-util.h"

#define SLOTS_NB 3
#define REPSIZE 500
#define REQSIZE 500

typedef enum {E_LIBRE, E_LIRE_REQUETE, E_ECRIRE_REQUETE} Etat;

typedef struct  {
    Etat etat;
    int soc;
    struct sockaddr_in adr;
    char req[REQSIZE];
    int req_pos;
    int fin_entete;
    char rep[REPSIZE];
    int rep_pos;
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
    o->req[0] = '\0';
    o->req_pos = 0;
    o->fin_entete = REQSIZE;
    o->rep[0] = '\0';
    o->rep_pos = 0;
}

int lire_suite_requete(Slot *o)
{
    int k = bor_read_str(o->soc, o->req + o->req_pos, REQSIZE - o->req_pos);
    if (k < 0) {
        return k;
    }
    o->req_pos += k;
    return k;
}

int ecrire_suite_requete(Slot *o)
{
    int k = bor_write_str(o->soc, o->rep + o->rep_pos);
    if (k < 0) {
        return k;
    }
    o->rep_pos += k;
    return k;
}

int chercher_fin_entete(Slot *o, int debut)
{
    int i = debut;
    int res = -1;
    for (; o->req[i] == '\0'; ++i) {
        if ((o->req[i] == '\n' && o->req[i+1] == '\n') ||
                (o->req[i] == '\r' && o->req[i+1] == '\n' && o->req[i+2] == '\r' && o->req[i+3] == '\n')) {
            res = i;
            break;
        }
    }
    return res;
}

typedef enum {
    M_NONE,
    M_GET,
    M_TRACE,
} Id_methode;

typedef enum {
    C_OK             = 200,
    C_BAD_REQUEST    = 400,
    C_NOT_FOUND      = 404,
    C_METHOD_UNKNOWN = 501,
} Code_reponse;

typedef struct _Infos_entete {
    char methode[REQSIZE],
    url[REQSIZE],
    version[REQSIZE],
    chemin[REQSIZE];
    Id_methode id_meth;
    Code_reponse code_rep;
} Infos_entete;

char *get_http_error_message(Code_reponse code)
{
    char *message; 
    switch (code) 
    {
        case C_OK :
            message = "OK";
            break;
        case C_BAD_REQUEST :
            message = "Bad request";
            break;
        case C_NOT_FOUND :
            message = "Not Found";
            break;
        case C_METHOD_UNKNOWN :
            message = "Method unknown";
            break;
    }
    return message;
}

Id_methode get_id_methode(char *methode)
{
    if (!strcasecmp(methode, "GET")) {
        return M_GET;
    }
    else if (!strcasecmp(methode, "TRACE")) {
        return M_TRACE; 
    }
    return M_NONE;
}

void analyser_requete(Slot *o, Infos_entete *ie)   
{
    (void) o;
    ie->code_rep = C_NOT_FOUND;
    ie->methode[0] = '\0';
    ie->url[0]     = '\0';
    ie->chemin[0]  = '\0';
    ie->version[0] = '\0';
    int i = 0;
    for (i = 0; o->req[i] != ' '; i++) {
        ie->methode[i] = o->req[i];
    }
    int j = i;
    for (; o->req[i] != ' '; i++) {
        ie->url[i - j] = o->req[i];
    }
    int k = 0;
    for (; ie->url[k] != '?'; k++) {
        ie->chemin[k] = o->req[k];
    }
    j = i;
    for (; o->req[i] != ' '; i++) {
        ie->version[i - j] = o->req[i];
    }
    ie->id_meth = get_id_methode(ie->methode);
    if (ie->methode[0] == '\0' || ie->url[0] == '\0' || ie->chemin[0] == '\0' || ie->version[0] == '\0') {
        ie->code_rep = C_BAD_REQUEST;
    }
    if (ie->id_meth == M_GET || ie->id_meth == M_TRACE) {
        ie->code_rep = C_OK;
    }
    else {
        ie->code_rep = C_METHOD_UNKNOWN;
    }
}

void preparer_reponse(Slot *o, Infos_entete *ie)
{
    if (ie->code_rep == C_NOT_FOUND)
        sprintf(o->rep, "HTTP/1.1 404 Not Found\n""server : serweb1\n""connection : close\n""content-type : text\\html\n"
                "\n""<html><head>\n""<title>Not Found</titles>\n""</head><body>\n""<h1>File not found</h1>\n"
                "</body></html>\n");
    else if (ie->code_rep == C_BAD_REQUEST)
        sprintf(o->rep, "HTTP/1.1 400 Bad Request\n""server : serweb1\n""connection : close\n""content-type : text\\html\n"
                "\n""<html><head>\n""<title>Bad Request</titles>\n""</head><body>\n""<h1>Bad Request</h1>\n"
                "</body></html>\n");
    else if (ie->code_rep == C_METHOD_UNKNOWN)
        sprintf(o->rep, "HTTP/1.1 501 Method Unknown\n""server : serweb1\n""connection : close\n""content-type : text\\html\n"
                "\n""<html><head>\n""<title>Method Unknown</titles>\n""</head><body>\n""<h1>Method Unknown</h1>\n"
                "</body></html>\n");
    else if (ie->code_rep == C_OK)
        sprintf(o->rep, "HTTP/1.1 200 OK\n""server : serweb1\n""connection : close\n""content-type : text\\html\n"
                "\n""<html><head>\n""<title>Welcome</titles>\n""</head><body>\n""<h1>IT WORKS</h1><br/><h3>My time machine works \\o/</h3><br/><h1>*BOOM*</h1><br/><h1>I AM A STEAKOSAURUS</h1>\n""</body></html>\n");
}

int slot_est_libre(Slot *o)
{
    return o->etat == E_LIBRE;
}

void liberer_slot(Slot *o)
{
    if (slot_est_libre(o)) return;
    close(o->soc);
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
    int prec_pos = o->req_pos;
    int k = lire_suite_requete(o);
    if (k <= 0) {
        return -1;
    }
    int debut = prec_pos - 3;
    if (debut < 0) {
        debut = 0;
    }
    o->fin_entete = chercher_fin_entete(o, debut);
    if (o->fin_entete < 0) {
        printf("Serveur[%d] : requete incomplete\n", o->soc);
        return 1;
    }
    printf("Serveur[%d] : recu requete\"%s\"\n", o->soc, o->req);
    Infos_entete ie;
    analyser_requete(o, &ie);
    preparer_reponse(o, &ie);
    o->etat = E_ECRIRE_REQUETE;
    return 1;
}

int proceder_ecrire_reponse(Slot *o)
{
    int k = ecrire_suite_requete(o); 
    if (k < 0) {
        return -1;
    }
    if (o->rep_pos < (int) strlen(o->rep)) {
        printf("Serveur[%d] : reponse incomplete\n", o->soc);
        return 1;
    }
    return 0;
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
