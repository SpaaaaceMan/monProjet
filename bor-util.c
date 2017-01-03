/* bor-util.c : Boite a Outil Reseau
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 10/12/2013 - V 3.1
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include "bor-util.h"


/* Affiche le message de perror en conservant la valeur de errno.
*/
void bor_perror (const char *funcname)
{
    int e = errno; perror (funcname); errno = e;
}


/*  Place le handler de signal void h(int) pour le signal sig avec sigaction().
 *  Le signal est automatiquement masque pendant sa delivrance.
 *  Si options = 0,
 *    - les appels bloquants sont interrompus avec retour -1 et errno = EINTR.
 *    - le handler est rearme automatiquement apres chaque delivrance de signal.
 *  si options est une combinaison bit a bit de
 *    - SA_RESTART : les appels bloquants sont silencieusement repris.
 *    - SA_RESETHAND : le handler n'est pas rearme.
 *  Renvoie le resultat de sigaction. Verbeux.
*/
int bor_signal (int sig, void (*h)(int), int options)
{
    int r;
    struct sigaction s;
    s.sa_handler = h;
    sigemptyset (&s.sa_mask);
    s.sa_flags = options;
    r = sigaction (sig, &s, NULL);
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Lecture d'au plus count octets dans fd et memorisation dans buf.
 * Renvoie le resultat de read(). Verbeux.
*/
ssize_t bor_read (int fd, void *buf, size_t count)
{
    ssize_t r = read (fd, buf, count);
    if (r < 0) bor_perror (__func__);
    else if (r == 0 && count != 0)
        fprintf (stderr, "%s: EOF detected for fd %d\n", __func__, fd);
    return r;
}


/* Lecture d'au plus count-1 caracteres dans fd et memorisation dans buf.
 * Ajout d'un '\0' terminal dans buf en cas de succes.
 * Renvoie le resultat de read(). Verbeux.
*/
ssize_t bor_read_str (int fd, char *buf, size_t count)
{
    if (count == 0) {
        fprintf (stderr, "%s: count>0 expected\n", __func__);
        errno = EINVAL;
        return -1;
    }
    ssize_t r = bor_read (fd, buf, count-1);
    if (r >= 0) buf[r] = '\0';
    return r;
}


/* Ecriture d'au plus count octets dans fd provenant de buf.
 * Renvoie le resultat de write(). Verbeux.
*/
ssize_t bor_write (int fd, const void *buf, size_t count)
{
    ssize_t r = write (fd, buf, count);
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Ecriture de la chaine de caracteres buf dans fd.
 * Renvoie le resultat de write(). Verbeux.
*/
ssize_t bor_write_str (int fd, const char *buf)
{
    return bor_write (fd, buf, strlen (buf));
}


/* Transforme la socket ouverte soc de type SOCK_STREAM en socket d'ecoute.
 * max_pending est la taille de la file des connexions en attente.
 * Renvoie le resultat de listen(). Verbeux.
*/
int bor_listen (int soc, int max_pending)
{
    int r = listen (soc, max_pending);
    if (r < 0) bor_perror (__func__);
    return r;
}


/*----------------------  D O M A I N E   U N I X ---------------------------*/


/* Cree une socket du domaine AF_UNIX selon le type, fabrique une adresse 
 * locale sa avec path puis attache la socket a sa.
 * Renvoie la socket attachee >= 0, sinon -1 erreur. Verbeux.
*/
int bor_create_socket_un (int type, const char *path, struct sockaddr_un *sa)
{
    int soc = socket (AF_UNIX, type, 0);
    if (soc < 0) { bor_perror ("socket un"); return -1; }

    bor_set_sockaddr_un (path, sa);
    if (bor_bind_un (soc, sa) < 0) { close (soc); return -1; }

    fprintf (stderr, "%s: socket \"%s\" opened\n", __func__, sa->sun_path);
    return soc;
}


/* Construit une adresse sa du domaine AF_UNIX avec le chemin path.
 * Silencieux.
*/
void bor_set_sockaddr_un (const char *path, struct sockaddr_un *sa)
{
    sa->sun_family = AF_UNIX;
    strncpy (sa->sun_path, path, UNIX_PATH_MAX);
    sa->sun_path[UNIX_PATH_MAX-1] = 0;
}


/* Attachement d'une socket de domaine AF_UNIX a une adresse sockaddr_un.
 * Renvoie le resultat de bind(). Verbeux.
*/
int bor_bind_un (int soc, struct sockaddr_un *sa)
{
    int r = bind (soc, (struct sockaddr *) sa, sizeof(struct sockaddr_un));
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Connexion a un serveur TCP/UN.
 * Renvoie le resultat de connect. Verbeux.
*/
int bor_connect_un (int soc, const struct sockaddr_un *sa)
{
    int r = connect (soc, (struct sockaddr *) sa, sizeof(struct sockaddr_un));
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Accepte une connexion en attente.
 * Renvoie le resultat de accept. Verbeux.
 */
int bor_accept_un (int soc, struct sockaddr_un *sa)
{
    socklen_t adrlen = sizeof(struct sockaddr_un);
    int r = accept (soc, (struct sockaddr *) sa, &adrlen);
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Reception d'un datagramme d'au plus count octets.
 * Renvoie le resultat de recvfrom. Verbeux.
*/
ssize_t bor_recvfrom_un (int soc, void *buf, size_t count, struct sockaddr_un *sa)
{
    socklen_t adrlen = sizeof(struct sockaddr_un);
    ssize_t  r = recvfrom (soc, buf, count, 0, (struct sockaddr *) sa, &adrlen);
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Reception d'un datagramme d'au plus count-1 caracteres.
 * Ajout d'un '\0' terminal dans buf en cas de succes.
 * Renvoie le resultat de read(). Verbeux.
*/
ssize_t bor_recvfrom_un_str (int soc, char *buf, size_t count, struct sockaddr_un *sa)
{
    ssize_t r = bor_recvfrom_un (soc, buf, count-1, sa);
    if (r >= 0) buf[r] = '\0';
    return r;
}


/* Envoi d'un datagramme.
 * Renvoie le resultat de sendto. Verbeux.
*/
ssize_t bor_sendto_un (int soc, const void *buf, size_t count, const struct sockaddr_un *sa)
{
    ssize_t r = sendto (soc, buf, count, 0, (struct sockaddr *) sa,
        sizeof(struct sockaddr_un));
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Envoi par datagramme de la chaine de caracteres buf dans soc.
 * Renvoie le resultat de sendto(). Verbeux.
*/
ssize_t bor_sendto_un_str (int soc, const void *buf, const struct sockaddr_un *sa)
{
    return bor_sendto_un (soc, buf, strlen (buf), sa);
}


/*-------------------  D O M A I N E   I N T E R N E T ----------------------*/


/* Cree une socket du domaine AF_INET selon le type, fabrique une adresse locale
 * sa avec le port (0 pour obtenir un port libre), puis attache la socket a sa.
 * Renvoie la socket attachee >= 0, sinon -1 erreur. Verbeux.
*/
int bor_create_socket_in (int type, int port, struct sockaddr_in *sa)
{
    int soc = socket (AF_INET, type, 0);
    if (soc < 0) { bor_perror ("socket in"); return -1; }
    
    bor_set_sockaddr_in (port, INADDR_ANY, sa);
    if (bor_bind_in (soc, sa) < 0) { close (soc); return -1; }

    if (bor_getsockname_in (soc, sa) < 0) { close (soc); return -1; }
    fprintf (stderr, "%s: port %d opened\n", __func__, ntohs(sa->sin_port));
    return soc;
}


/* Construit une adresse sa du domaine AF_INET avec le port et l'adresse ipv4.
 * Donner port = 0 pour obtenir un port libre ; ipv4 = INADDR_ANY pour l'adresse 
 * locale. Silencieux.
*/
void bor_set_sockaddr_in (int port, uint32_t ipv4, struct sockaddr_in *sa)
{
    sa->sin_family = AF_INET;
    sa->sin_port = htons (port);
    sa->sin_addr.s_addr = htonl(ipv4);
}


/* Recuperation de l'adresse reelle et du port sous forme Network.
 * Renvoie le resultat de getsockname. Verbeux.
*/
int bor_getsockname_in (int soc, struct sockaddr_in *sa)
{
    socklen_t adrlen = sizeof(struct sockaddr_in);
    int r = getsockname (soc, (struct sockaddr *) sa, &adrlen);
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Renvoie l'adresse d'une chaine en memoire statique contenant l'adresse IPv4
 * sous la forme "a.b.c.d:port", de facon a pouvoir l'afficher. Silencieux.
*/
char *bor_adrtoa_in (struct sockaddr_in *sa)
{
    static char s[32];
    sprintf (s, "%s:%d", inet_ntoa (sa->sin_addr), ntohs (sa->sin_port));
    return s;
}


/* Attachement d'une socket de domaine AF_INET a une adresse sockaddr_in.
 * Renvoie le resultat de bind(). Verbeux.
*/
int bor_bind_in (int soc, struct sockaddr_in *sa)
{
    int r = bind (soc, (struct sockaddr *) sa, sizeof(struct sockaddr_in));
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Resout une adresse IPV4 sous la forme "nom_local" ou decimale "1.2.3.4"
 * ou FQDN "machine.domaine.tld", puis la stocke avec le port dans sa.
 * Renvoie 0 succes ou -1 erreur. Verbeux.
*/
int bor_resolve_address_in (const char *host, int port, struct sockaddr_in *sa)
{
    sa->sin_family = AF_INET;
    sa->sin_port = htons (port);

    struct hostent *hp;
    if ((hp = gethostbyname (host)) == NULL)
      { herror (__func__); return -1; }  
    memcpy (&sa->sin_addr.s_addr, hp->h_addr, hp->h_length);
    return 0;
}


/* Connexion a un serveur TCP/IP.
 * Renvoie le resultat de connect. Verbeux.
*/
int bor_connect_in (int soc, const struct sockaddr_in *sa)
{
    int r = connect (soc, (struct sockaddr *) sa, sizeof(struct sockaddr_in));
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Accepte une connexion en attente.
 * Renvoie le resultat de accept. Verbeux.
*/
int bor_accept_in (int soc, struct sockaddr_in *sa)
{
    socklen_t adrlen = sizeof(struct sockaddr_in);
    int r = accept (soc, (struct sockaddr *) sa, &adrlen);
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Reception d'un datagramme.
 * Renvoie le resultat de recvfrom. Verbeux.
*/
ssize_t bor_recvfrom_in (int soc, void *buf, size_t count, struct sockaddr_in *sa)
{
    socklen_t adrlen = sizeof(struct sockaddr_in);
    ssize_t r = recvfrom (soc, buf, count, 0, (struct sockaddr *) sa, &adrlen);
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Reception d'un datagramme d'au plus count-1 caracteres.
 * Ajout d'un '\0' terminal dans buf en cas de succes.
 * Renvoie le resultat de read(). Verbeux.
*/
ssize_t bor_recvfrom_in_str (int soc, char *buf, size_t count, struct sockaddr_in *sa)
{
    ssize_t r = bor_recvfrom_in (soc, buf, count-1, sa);
    if (r >= 0) buf[r] = '\0';
    return r;
}


/* Envoi d'un datagramme.
 * Renvoie le resultat de sendto. Verbeux.
*/
ssize_t bor_sendto_in (int soc, const void *buf, size_t count, const struct sockaddr_in *sa)
{
    ssize_t r = sendto (soc, buf, count, 0, (struct sockaddr *) sa,
        sizeof(struct sockaddr_in));
    if (r < 0) bor_perror (__func__);
    return r;
}


/* Envoi par datagramme de la chaine de caracteres buf dans soc.
 * Renvoie le resultat de sendto(). Verbeux.
*/
ssize_t bor_sendto_in_str (int soc, const void *buf, const struct sockaddr_in *sa)
{
    return bor_sendto_in (soc, buf, strlen (buf), sa);
}


