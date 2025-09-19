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
    FILE *input = stdin;
    int buffer_size = 4096;
    char *filename = NULL;
    int opt;
    char *buffer;
    size_t bytes_read;
    long total_bytes = 0, total_lines = 0;

    clock_t start = clock();

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = handle_sigusr1;
    sigaction(SIGUSR1, &sa, NULL);

    while ((opt = getopt(argc, argv, "f:b:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'b':
                buffer_size = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-f filename] [-b buffer_size]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (filename) {
        input = fopen(filename, "r");
        if (!input) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }

    buffer = malloc(buffer_size);
    if (!buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (!shutdown_flag && (bytes_read = fread(buffer, 1, buffer_size, input)) > 0) {
        fwrite(buffer, 1, bytes_read, stdout);
        total_bytes += bytes_read;

        for (size_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') total_lines++;
        }

        if (stats_flag) {
            fprintf(stderr, "[Producer] So far: %ld lines, %ld bytes\n", total_lines, total_bytes);
            stats_flag = 0;
        }
    }

    if (input != stdin) fclose(input);
    free(buffer);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    double throughput = (elapsed > 0) ? (total_bytes / elapsed) : 0;

    fprintf(stderr, "[Producer] Finished: %ld lines, %ld bytes\n", total_lines, total_bytes);
    fprintf(stderr, "[Producer] Time elapsed: %.3f sec, Throughput: %.2f bytes/sec\n", elapsed, throughput);

    return 0;
}
