#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    
    // Parsing the CLI arguments
    if (argc != 3) {
        fprintf(stderr, "Incorrect program execution. Expected arguments: \"./%s dx n\", where dx is the width of the rectangle, and n is the maximum number of processes.\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *endptr;
    const double dx = strtod(argv[1], &endptr);

    if (endptr == argv[1] || *endptr != '\0') {
        fprintf(stderr, "Incorrect program execution. Expected arguments: \"./%s dx n\", where dx is the width of the rectangle, and n is the maximum number of processes.\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const long n = strtol(argv[2], &endptr, 10);

    if (endptr == argv[2] || *endptr != '\0') {
        fprintf(stderr, "Incorrect program execution. Expected arguments: \"./%s dx n\", where dx is the width of the rectangle, and n is the maximum number of processes.\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Main body of the program
    for (int k = 1; k <= n; ++k) {
        // TODO
    }

    return EXIT_SUCCESS;
}