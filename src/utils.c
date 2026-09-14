#include "../header/ft_ping.h"

void print_help(void) {
    printf("Usage: ft_ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n");
    printf("  -v                         verbose output\n");
    printf("  -?                         give this help list\n");
}

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
