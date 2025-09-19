#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    int max_lines = -1;   // -1 means unlimited
    int verbose = 0;
    int opt;

    // parse arguments: -n <max_lines>, -v (verbose)
    while ((opt = getopt(argc, argv, "n:v")) != -1) {
        switch (opt) {
            case 'n': {
                char *endptr = NULL;
                errno = 0;
                long val = strtol(optarg, &endptr, 10);
                if (errno != 0 || endptr == optarg || *endptr != '\0' || val < 0) {
                    fprintf(stderr, "Invalid value for -n: %s\n", optarg);
                    return 2;
                }
                max_lines = (int) val;
                break;
            }
            case 'v':
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [-n max_lines]\n", argv[0]);
                return 2;
        }
    }

    // Read from stdin line by line
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    long line_count = 0;
    long char_count = 0;

    // If max_lines == 0, we should not read any lines.
    while ((linelen = getline(&line, &linecap, stdin)) != -1) {
        // Count line and characters
        line_count++;
        char_count += linelen; // getline includes the newline if present

        if (verbose) {
            // write the same bytes to stdout
            if (fwrite(line, 1, (size_t)linelen, stdout) != (size_t)linelen) {
                perror("fwrite");
                free(line);
                return 3;
            }
            fflush(stdout);
        }

        if (max_lines != -1 && line_count >= max_lines) {
            break;
        }
    }

    // clean up
    free(line);

    // Print statistics to stderr
    fprintf(stderr, "Lines: %ld\n", line_count);
    fprintf(stderr, "Chars: %ld\n", char_count);

    return 0;
}
