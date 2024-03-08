#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <part_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sleep(10);

    const char* partFile = argv[1];
    char resultFilename[260];
    sprintf(resultFilename, "%s_result", partFile);

    FILE *file = fopen(partFile, "r");
    if (!file) {
        fprintf(stderr, "Error opening input file: %s\n", partFile);
        exit(EXIT_FAILURE);
    }

    double sum = 0;
    double number;
    while (fscanf(file, "%lf", &number) == 1) {
        sum += number * number;
    }
    fclose(file);

    file = fopen(resultFilename, "w");
    if (!file) {
        fprintf(stderr, "Error creating result file: %s\n", resultFilename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%lf", sum);
    fclose(file);

    return 0;
}
