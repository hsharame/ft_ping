#include "../header/ft_ping.h"

t_ping g_ping;

int main(int argc, char *argv[]) {
    memset(&g_ping, 0, sizeof(t_ping));
    g_ping.pid = (uint16_t)getpid();
    g_ping.min_rtt = -1.0; // no iteration yet

    // TODO verifier la syntaxe et l'ordre des flags, test several hosts
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v")) {
            g_ping.verbose = 1;
        } else if (strcmp(argv[i], "-?")) {
            print_help();
            return 0;
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

    // TODO dans man Le comportement de signal() varie selon les versions d'Unix, 
    // et a également varié au cour du temps dans les différentes versions de Linux. 
    // Évitez son utilisation : utilisez plutôt sigaction(2). 
    signal(SIGINT, print_stats);

    init_socket(&g_ping);

    if (g_ping.verbose)
    	printf("PING %s (%s): %ld data bytes, id 0x%04x = %d\n", 
        g_ping.target_host, g_ping.dest_ip, PAYLOAD_SIZE, g_ping.pid, g_ping.pid);
    else
    	printf("PING %s (%s): %ld data bytes\n", 
        g_ping.target_host, g_ping.dest_ip, PAYLOAD_SIZE);    // Lancement du timer global et de la boucle
    
    gettimeofday(&g_ping.start_time, NULL);
    loop_ping(&g_ping);

    return 0;
}
