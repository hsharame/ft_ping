#include "../header/ft_ping.h"

void send_ping(t_ping *ping_data) {
    char packet[PACKET_SIZE];
    memset(packet, 0, sizeof(packet));

    // Construction du header ICMP
    struct icmphdr *icmp = (struct icmphdr *)packet;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(ping_data->pid);
    ping_data->seq++;
    icmp->un.echo.sequence = htons(ping_data->seq);

    // Injection du timestamp dans le payload pour calculer le RTT à la réception
    struct timeval *time_payload = (struct timeval *)(packet + sizeof(struct icmphdr));
    gettimeofday(time_payload, NULL);

    // Calcul du checksum sur l'ensemble du paquet (header + payload)
    icmp->checksum = calculate_checksum(packet, PACKET_SIZE);

    if (sendto(ping_data->sockfd, packet, PACKET_SIZE, 0, 
              (struct sockaddr *)&ping_data->dest_addr, sizeof(ping_data->dest_addr)) <= 0) {
        fprintf(stderr, "ft_ping: sendto error\n");
    } else {
        ping_data->packets_sent++;
    }
}

void receive_ping(t_ping *ping_data) {
    char buffer[1024];
    struct sockaddr_in r_addr;
    socklen_t addr_len = sizeof(r_addr);
    struct timeval recv_time;

    // Timeout de 1 seconde pour recvfrom (évite le blocage si perte de paquet)
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(ping_data->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    ssize_t ret = recvfrom(ping_data->sockfd, buffer, sizeof(buffer), 0, 
                           (struct sockaddr *)&r_addr, &addr_len);
    gettimeofday(&recv_time, NULL);

    if (ret > 0) {
        // Le buffer contient le paquet IP complet
        struct iphdr *ip = (struct iphdr *)buffer;
        int ip_hdr_len = ip->ihl * 4;
        
        // Le paquet ICMP commence après le header IP
        struct icmphdr *icmp_reply = (struct icmphdr *)(buffer + ip_hdr_len);

        // Conversion stricte de l'IP reçue, SANS appel à gethostbyaddr (Pas de résolution DNS inversée)
        char src_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ip->saddr), src_ip, INET_ADDRSTRLEN);

        if (icmp_reply->type == ICMP_ECHOREPLY) {
            // Vérifier qu'il s'agit bien de notre paquet
            if (ntohs(icmp_reply->un.echo.id) == ping_data->pid) {
                struct timeval *send_time = (struct timeval *)(buffer + ip_hdr_len + sizeof(struct icmphdr));
                double rtt = get_time_ms(send_time, &recv_time);

                ping_data->packets_received++;
                ping_data->sum_rtt += rtt;
                if (ping_data->min_rtt < 0 || rtt < ping_data->min_rtt)
                    ping_data->min_rtt = rtt;
                if (rtt > ping_data->max_rtt)
                    ping_data->max_rtt = rtt;
		ping_data->sum_sq_rtt += rtt * rtt;
                printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
                       ret - ip_hdr_len, src_ip, ntohs(icmp_reply->un.echo.sequence), ip->ttl, rtt);
            }
        } else {
            if (ping_data->verbose) {
                printf("%ld bytes from %s: type = %d, code = %d\n",
                       ret - ip_hdr_len, src_ip, icmp_reply->type, icmp_reply->code);
            }
        }
    }
}

void loop_ping(t_ping *ping_data) {
    while (1) {
        send_ping(ping_data);
        receive_ping(ping_data);
        sleep(1);
    }
}
