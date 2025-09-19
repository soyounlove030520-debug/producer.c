#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

volatile sig_atomic_t shutdown_flag = 0;
volatile sig_atomic_t stats_flag = 0;

void handle_sigint(int sig) {
    shutdown_flag = 1;
}

void handle_sigusr1(int sig) {
    stats_flag = 1;
}

int main(int argc, char *argv[]) {
    int max_lines = -1;
    int verbose = 0;
    int opt;
    char buffer[4096];
    long line_count = 0, char_count = 0;

    clock_t start = clock();

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = handle_sigusr1;
    sigaction(SIGUSR1, &sa, NULL);

    while ((opt = getopt(argc, argv, "n:v")) != -1) {
        switch (opt) {
            case 'n':
                max_lines = atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n max_lines] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    while (!shutdown_flag && fgets(buffer, sizeof(buffer), stdin)) {
        line_count++;
        char_count += strlen(buffer);

        if (verbose) {
            fputs(buffer, stdout);
        }

        if (stats_flag) {
            fprintf(stderr, "[Consumer] So far: %ld lines, %ld chars\n", line_count, char_count);
            stats_flag = 0;
        }

        if (max_lines != -1 && line_count >= max_lines) {
            break;
        }
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    double throughput = (elapsed > 0) ? (char_count / elapsed) : 0;

    fprintf(stderr, "[Consumer] Finished: %ld lines, %ld chars\n", line_count, char_count);
    fprintf(stderr, "[Consumer] Time elapsed: %.3f sec, Throughput: %.2f chars/sec\n", elapsed, throughput);

    return 0;
}
