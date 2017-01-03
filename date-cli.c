#include "bor-util.h"

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
    bor_set_sockaddr_un(path_serv, &addr_serv);
    int soc = bor_create_socket_un(SOCK_DGRAM, path, &addr);
    int co = bor_connect_un(soc, &addr_serv);
    if (co < 0) {
        goto fin1;
    }
    char buf[1000];
    sprintf(buf, "HELLO !!!");
    int sd = bor_sendto_un_str(soc, buf, &addr_serv);
    if (sd < 0) {
        goto fin1;
    }
    int rc = bor_recvfrom_un_str(soc, buf, sizeof(buf), &addr_serv);
    if (rc < 0) {
        goto fin1;
    }
    printf("%s\n", buf);
fin1:
    close(soc);
    unlink(addr.sun_path);
    return 0;
}
