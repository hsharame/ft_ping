#include "../header/ft_ping.h"

t_ping g_ping;

int main(int argc, char *argv[]) {
    int val;
    memset(&g_ping, 0, sizeof(t_ping));
    g_ping.pid = (uint16_t)getpid();
    g_ping.min_rtt = -1.0; // no iteration yet

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            g_ping.verbose = 1;
        } else if (strcmp(argv[i], "-?") == 0) {
            print_help();
            return 0;
        } else if (strncmp(argv[i], "--ttl=", 6) == 0) {
            val = atoi(argv[i] + 6);
            if (val <= 0 || val > 255) {
                fprintf(stderr, "ft_ping: option value too big: %d\n", val);
                return 1;
            }
            g_ping.ttl = val;
        } else if (strcmp(argv[i], "--ttl") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ft_ping: option '--ttl' requires an argument\n");
                return 1;
            }
            val = atoi(argv[++i]);
            if (val <= 0 || val > 255) {
                fprintf(stderr, "ft_ping: option value too big: %d\n", val);
                return 1;
            }
            g_ping.ttl = val;
        } else if (argv[i][0] != '-') {
            if (g_ping.target_host == NULL)
                g_ping.target_host = argv[i];
        } else {
            fprintf(stderr, "ft_ping: invalid option -- '%s'\n", argv[i]);
            fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
            return 1;
        }
    }
    if (!g_ping.target_host) {
        fprintf(stderr, "ft_ping: missing host operand\n");
        fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
        return 1;
    }

    // signal(SIGINT, print_stats);
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = print_stats;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("ft_ping: sigaction failed");
        exit(1);
    }

    init_socket(&g_ping);

    if (g_ping.verbose)
    	printf("PING %s (%s): %ld data bytes, id 0x%04x = %d\n", 
        g_ping.target_host, g_ping.dest_ip, PAYLOAD_SIZE, g_ping.pid, g_ping.pid);
    else
    	printf("PING %s (%s): %ld data bytes\n", 
        g_ping.target_host, g_ping.dest_ip, PAYLOAD_SIZE);

    loop_ping(&g_ping);

    return 0;
}
