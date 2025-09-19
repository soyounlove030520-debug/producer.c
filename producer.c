#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    FILE *input = stdin;         
    int buffer_size = 4096;     
    char *filename = NULL;
    int opt;

    // --- Parse command line arguments ---
    while ((opt = getopt(argc, argv, "f:b:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;   // -f filename
                break;
            case 'b':
                buffer_size = atoi(optarg); // -b buffer_size
                if (buffer_size <= 0) {
                    fprintf(stderr, "Invalid buffer size: %s\n", optarg);
                    return 1;
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-f filename] [-b buffer_size]\n", argv[0]);
                return 1;
        }
    }

    // --- Open file if -f provided ---
    if (filename != NULL) {
        input = fopen(filename, "r");
        if (!input) {
            perror("fopen");
            return 1;
        }
    }

    // --- Allocate buffer ---
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        if (filename != NULL) fclose(input);
        return 1;
    }

    // --- Read from input and write to stdout ---
    size_t n;
    while ((n = fread(buffer, 1, buffer_size, input)) > 0) {
        fwrite(buffer, 1, n, stdout);
    }

    // --- Cleanup ---
    free(buffer);
    if (filename != NULL) fclose(input);

    return 0;
}
