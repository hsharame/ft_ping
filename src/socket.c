#include "../header/ft_ping.h"


// struct addrinfo {
//     int              ai_flags;
//     int              ai_family;
//     int              ai_socktype;
//     int              ai_protocol;
//     size_t           ai_addrlen;
//     struct sockaddr *ai_addr;
//     char            *ai_canonname;
//     struct addrinfo *ai_next;
// };

void init_socket(t_ping *ping_data) {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    // Resolve target host without port using hints criteria (IPv4/RAW/ICMP) and store allocated results in 'res'
    if (getaddrinfo(ping_data->target_host, NULL, &hints, &res) != 0) {
        fprintf(stderr, "ft_ping: unknown host %s\n", ping_data->target_host);
        exit(1);
    }

    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    memcpy(&ping_data->dest_addr, addr, sizeof(struct sockaddr_in));
    // convert IPv4 address from binary to text form
    inet_ntop(AF_INET, &(addr->sin_addr), ping_data->dest_ip, INET_ADDRSTRLEN);
    freeaddrinfo(res);

    ping_data->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (ping_data->sockfd < 0) {
        fprintf(stderr, "ft_ping: lacking privilege for raw socket\n");
        exit(1);
    }
}
