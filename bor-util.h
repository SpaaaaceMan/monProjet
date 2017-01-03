/* bor-util.h : Boite a Outil Reseau
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 10/12/2013 - V 3.1
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#ifndef BOR_UTIL__H
#define BOR_UTIL__H

/* Compatibilite avec -ansi */
#if defined __STRICT_ANSI__ && !defined _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     /* exit */
#include <unistd.h>     /* close */
#include <string.h>     /* strlen */
#include <sys/types.h>  /* open, socket, bind, sendto, recvfrom, wait */
#include <signal.h>     /* sigaction */
#include <sys/wait.h>   /* wait */
#include <sys/stat.h>   /* open */
#include <fcntl.h>      /* open */
#include <sys/socket.h> /* socket, bind, sendto, recvfrom, getsockname */
#include <sys/un.h>     /* socket domaine AF_UNIX */
#include <netinet/ip.h> /* socket domaine AF_INET */
#include <arpa/inet.h>  /* inet_ntoa */
#include <netdb.h>      /* gethostbyname */
#include <sys/time.h>   /* gettimeofday */
#include <time.h>       /* time, gettimeofday */
#include <errno.h>      /* errno */

/* Compatibilite avec C++ */
#if defined(c_plusplus) && !defined(__cplusplus)
#define __cplusplus
#endif
#ifdef __cplusplus
extern "C" {
#endif


/* UNIX_PATH_MAX pas toujours definie */
#ifndef UNIX_PATH_MAX
#ifdef __APPLE__
#define UNIX_PATH_MAX 104
#else
#define UNIX_PATH_MAX 108
#endif
#endif

/* Prototypes */
void bor_perror (const char *funcname);
int bor_signal (int sig, void (*h)(int), int options);

ssize_t bor_read (int fd, void *buf, size_t count);
ssize_t bor_read_str (int fd, char *buf, size_t count);
ssize_t bor_write (int fd, const void *buf, size_t count);
ssize_t bor_write_str (int fd, const char *buf);

int bor_listen (int soc, int max_pending);

int bor_create_socket_un (int type, const char *path, struct sockaddr_un *sa);
void bor_set_sockaddr_un (const char *path, struct sockaddr_un *sa);
int bor_bind_un (int soc, struct sockaddr_un *sa);
int bor_connect_un (int soc, const struct sockaddr_un *sa);
int bor_accept_un (int soc, struct sockaddr_un *sa);
ssize_t bor_recvfrom_un (int soc, void *buf, size_t count, struct sockaddr_un *sa);
ssize_t bor_recvfrom_un_str (int soc, char *buf, size_t count, struct sockaddr_un *sa);
ssize_t bor_sendto_un (int soc, const void *buf, size_t count, const struct sockaddr_un *sa);
ssize_t bor_sendto_un_str (int soc, const void *buf, const struct sockaddr_un *sa);

int bor_create_socket_in (int type, int port, struct sockaddr_in *sa);
void bor_set_sockaddr_in (int port, uint32_t ipv4, struct sockaddr_in *sa);
int bor_getsockname_in (int soc, struct sockaddr_in *sa);
char *bor_adrtoa_in (struct sockaddr_in *sa);
int bor_bind_in (int soc, struct sockaddr_in *sa);
int bor_resolve_address_in (const char *host, int port, struct sockaddr_in *sa);
int bor_connect_in (int soc, const struct sockaddr_in *sa);
int bor_accept_in (int soc, struct sockaddr_in *sa);
ssize_t bor_recvfrom_in (int soc, void *buf, size_t count, struct sockaddr_in *sa);
ssize_t bor_recvfrom_in_str (int soc, char *buf, size_t count, struct sockaddr_in *sa);
ssize_t bor_sendto_in (int soc, const void *buf, size_t count, const struct sockaddr_in *sa);
ssize_t bor_sendto_in_str (int soc, const void *buf, const struct sockaddr_in *sa);


/* Solution de repli pour __func__ */
#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

/* Compatibilite avec C++ */
#ifdef __cplusplus
}
#endif

#endif /* BOR_UTIL__H */

