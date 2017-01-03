#include "bor-util.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char *tube_ec_nom, tube_cs_nom[100], tube_sc_nom[100];
    int tube_ec, tube_sc, tube_cs;
} Client;

int creer_tubes_service(Client *c){
    int k;
    printf("crétion des tubes services\n");
    sprintf(c->tube_sc_nom, "tub-sc-%d.tmp", (int) getpid());
    k = mkfifo(c->tube_sc_nom, 0600);
    if (k < 0) {
        perror("mkfifo tube_sc");
        unlink(c->tube_cs_nom);
        return k;
    }
    sprintf(c->tube_cs_nom, "tub-sc-%d.tmp", (int) getpid());
    k = mkfifo(c->tube_cs_nom, 0600);
    if (k < 0) {
        perror("mkfifo tube_cs");
        unlink(c->tube_cs_nom);
        return k;
    }
    return 0;
}

int supprimer_tubes_services(Client *c) {
    printf("suppression des tubes services\n");
    unlink(c->tube_cs_nom);
    unlink(c->tube_sc_nom);
}

int fermer_tubes_service(Client *c)
{
    printf("fermeture des tubes services\n");
    close(c->tube_cs);
    close(c->tube_sc);
}

int ouvrir_tubes_service(Client *c)
{
    printf("ouverture tube_sc\n");
    c->tube_sc = open(c->tube_sc_nom, O_RDONLY);
    if (c->tube_sc < 0) {
        perror("open tube_sc");
        return -1;
    }
    printf("ouverture tube_cs\n");
    c->tube_cs = open(c->tube_cs_nom, O_WRONLY);
    if (c->tube_cs < 0) {
        perror("open tube_cs");
        return -1;
    }
    return 0;
}

int prendre_contact(Client *c)
{
    char buf[100];
    printf("ouverture tube_ec\n");
    c->tube_ec = open(c->tube_ec_nom, O_WRONLY);
    if (c->tube_ec < 0) {
        perror("open tube_ec");
        return -1;
    }
    sprintf(buf, "%s %si", c->tube_sc_nom, c->tube_cs_nom);
    printf("Envoi noms au serveur...\n");
    int k = bor_write_str(c->tube_ec, buf);
    if (k < 0) {
        return k;
    }
    close(c->tube_ec);
    return 0;
}

int faire_dialogue(Client *c)
{
    char buf[100];
    printf("Entrez un nom de famille : \n");
    int k = bor_read_str(0, buf, sizeof(buf));
    if (k <= 0) {
        return k;
    }
    printf("Envoi au serveur...\n");
    k = bor_write_str(c->tube_cs, buf);
    if (k <= 0) {
        return k;
    }
    printf("Attente réponse...\n");
    k = bor_read_str(c->tube_sc, buf, sizeof(buf));
    if (k <= 0) {
        return k;
    }
    printf("Réponse : %s\n", buf);
    return 1;
}

int main (int argc, char* argv[]) {
    Client c;
    int r;
    if (argc != 2) {
        fprintf(stderr, "Un argument requis !");
        exit (1);
    }
    c.tube_ec_nom = argv[1];
    bor_signal(SIGPIPE, SIG_IGN, SA_RESTART);
    if (creer_tubes_service(&c) < 0) {
        exit(1);
    }
    if (prendre_contact(&c) < 0) {
        goto fin1; 
    }
    if (ouvrir_tubes_service(&c) < 0) {
        goto fin1;
    }
    while (1) {
        r = faire_dialogue(&c);
        if (r < 0) {
            break;
        }
    }
    fermer_tubes_service(&c);
fin1:
    supprimer_tubes_services(&c);
    exit(r < 0 ? 1 : 0);
    return 0;
}
