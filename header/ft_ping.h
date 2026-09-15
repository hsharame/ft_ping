#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <sys/time.h>
# include <signal.h>
# include <errno.h>
# include <math.h>

# define PACKET_SIZE 64
# define PAYLOAD_SIZE (PACKET_SIZE - sizeof(struct icmphdr)) // without header ICMP

typedef struct s_ping {
    int             verbose;
    char            *target_host;
    char            dest_ip[INET_ADDRSTRLEN];
    int             sockfd;
    struct sockaddr_in dest_addr;
    int             seq;
    int             packets_sent;
    int             packets_received;
    pid_t           pid;
    double          min_rtt;
    double          max_rtt;
    double          sum_rtt;
    double	        sum_sq_rtt;
}   t_ping;

extern t_ping g_ping;

void            print_help(void);
unsigned short  calculate_checksum(void *b, int len);
double          get_time_ms(struct timeval *start, struct timeval *end);
void            print_stats(int sig);

void            init_socket(t_ping *ping_data);
void            loop_ping(t_ping *ping_data);
void            send_ping(t_ping *ping_data);
void            receive_ping(t_ping *ping_data);

#endif
