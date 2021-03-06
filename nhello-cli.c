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
    if (soc < 0) {
        goto fin1;
    }
    int cpt = 10000;
    char buf[100];
    while (cpt > 0) {
        int sd = bor_sendto_un_str(soc, "HELLO", &addr_serv);
        if (sd < 0) {
            goto fin1;
        }
        --cpt;
    }
    int sd = bor_sendto_un_str(soc, "NUMBER", &addr_serv);    
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
