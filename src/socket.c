#include "../header/ft_ping.h"

void init_socket(t_ping *ping_data) {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Limité à IPv4 selon les consignes
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    // Résolution DNS du nom d'hôte (FQDN)
    if (getaddrinfo(ping_data->target_host, NULL, &hints, &res) != 0) {
        fprintf(stderr, "ft_ping: unknown host %s\n", ping_data->target_host);
        exit(1);
    }

    // Récupération et sauvegarde de l'adresse IP
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    memcpy(&ping_data->dest_addr, addr, sizeof(struct sockaddr_in));
    inet_ntop(AF_INET, &(addr->sin_addr), ping_data->dest_ip, INET_ADDRSTRLEN);
    freeaddrinfo(res);

    // Création du socket RAW (nécessite d'être root)
    ping_data->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (ping_data->sockfd < 0) {
        fprintf(stderr, "ft_ping: Lacking privilege for raw socket.\n");
        exit(1);
    }
}
