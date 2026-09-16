#include "../header/ft_ping.h"

unsigned short calculate_checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;
    
    for (sum = 0; len > 1; len -= 2)
    sum += *buf++;
    if (len == 1)
    sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

double get_time_ms(struct timeval *start, struct timeval *end) {
    double start_ms = (double)start->tv_sec * 1000.0 + (double)start->tv_usec / 1000.0;
    double end_ms = (double)end->tv_sec * 1000.0 + (double)end->tv_usec / 1000.0;
    return end_ms - start_ms;
}

void print_help(void) {
    printf("Usage: ft_ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n");
    printf("  -v                         verbose output\n");
    printf("  -?                         give this help list\n");
}

void print_stats(int sig) {
    (void)sig;
    int loss = 0;

    if (g_ping.packets_sent > 0) {
        loss = ((g_ping.packets_sent - g_ping.packets_received) * 100) / g_ping.packets_sent;
    }

    printf("\n--- %s ping statistics ---\n", g_ping.target_host);
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
           g_ping.packets_sent, g_ping.packets_received, loss);

    if (g_ping.packets_received > 0) {
        double avg = g_ping.sum_rtt / g_ping.packets_received;
        double variance = (g_ping.sum_sq_rtt / g_ping.packets_received) - (avg * avg);
        double stddev = sqrt(variance < 0 ? 0 : variance);

        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
               g_ping.min_rtt, avg, g_ping.max_rtt, stddev);
    }

    if (g_ping.sockfd > 0)
        close(g_ping.sockfd);

    exit(0);
}

// --ttl -v error
void print_verbose_time_exceeded(const uint8_t *buffer, ssize_t bytes_recvd) {
    struct iphdr *outer_ip = (struct iphdr *)buffer;
    int outer_ip_len = outer_ip->ihl * 4;
    struct icmphdr *icmp = (struct icmphdr *)(buffer + outer_ip_len);

    ssize_t min_len = outer_ip_len + sizeof(struct icmphdr) + sizeof(struct iphdr);
    if (bytes_recvd < min_len)
        return;

    struct iphdr *inner_ip = (struct iphdr *)((uint8_t *)icmp + sizeof(struct icmphdr));
    uint8_t *ip_ptr = (uint8_t *)inner_ip;
    int inner_ip_len = inner_ip->ihl * 4;

    printf("IP Hdr Dump:\n ");
    for (int i = 0; i < inner_ip_len; i += 2) {
        printf("%02x%02x ", ip_ptr[i], ip_ptr[i + 1]);
    }
    printf("\n");

    char src_str[INET_ADDRSTRLEN];
    char dst_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(inner_ip->saddr), src_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(inner_ip->daddr), dst_str, INET_ADDRSTRLEN);

    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
    printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %s  %s\n",
           inner_ip->version,
           inner_ip->ihl,
           inner_ip->tos,
           ntohs(inner_ip->tot_len),
           ntohs(inner_ip->id),
           (ntohs(inner_ip->frag_off) >> 13) & 0x7,
           ntohs(inner_ip->frag_off) & 0x1FFF,
           inner_ip->ttl,
           inner_ip->protocol,
           ntohs(inner_ip->check),
           src_str,
           dst_str);

    if (bytes_recvd >= min_len + sizeof(struct icmphdr)) {
        struct icmphdr *inner_icmp = (struct icmphdr *)((uint8_t *)inner_ip + inner_ip_len);
        printf("ICMP: type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n",
               inner_icmp->type,
               inner_icmp->code,
               ntohs(inner_ip->tot_len) - inner_ip_len,
               ntohs(inner_icmp->un.echo.id),
               ntohs(inner_icmp->un.echo.sequence));
    }
}