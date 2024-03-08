#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

#define BUFFER_SIZE 1024

void DistributeDataAmongChildren(const char* filename, int nChildren, int totalNumbers) {
    FILE* inFile = fopen(filename, "r");
    if (!inFile) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int numbersPerChild = totalNumbers / nChildren;
    int extraNumbers = totalNumbers % nChildren;

    for (int i = 0; i < nChildren; i++) {
        char childFilename[BUFFER_SIZE];
        sprintf(childFilename, "%s_part_%d", filename, i);

        FILE* childFile = fopen(childFilename, "w");
        if (!childFile) {
            fprintf(stderr, "Error creating file for child process: %s\n", childFilename);
            fclose(inFile);
            exit(EXIT_FAILURE);
        }

        int limit = numbersPerChild + (i < extraNumbers ? 1 : 0);
        for (int j = 0; j < limit; j++) {
            double number;
            if (fscanf(inFile, "%lf", &number) != 1) break;
            fprintf(childFile, "%lf\n", number);
        }
        fclose(childFile);
    }
    fclose(inFile);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <number_of_children>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];
    long long desiredChildren = atoll(argv[2]);

    if (desiredChildren <= 0 || desiredChildren > INT_MAX) {
        fprintf(stderr, "Error: Number of children is out of valid range (1 to %d).\n", INT_MAX);
        exit(EXIT_FAILURE);
    }

    FILE* inFile = fopen(filename, "r");
    if (!inFile) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int totalNumbers = 0;
    double number;
    while (fscanf(inFile, "%lf", &number) == 1) totalNumbers++;
    fclose(inFile);

    if (totalNumbers < 2) {
        fprintf(stderr, "Error: The file must contain at least two numbers.\n");
        exit(EXIT_FAILURE);
    }

    int nChildren = desiredChildren > totalNumbers ? totalNumbers : desiredChildren;
    if (desiredChildren > nChildren) {
        printf("Warning: Requested number of child processes reduced to %d due to insufficient data.\n", nChildren);
    } else {
        printf("Spawning %d child processes...\n", nChildren);
    }

    DistributeDataAmongChildren(filename, nChildren, totalNumbers);

    for (int i = 0; i < nChildren; i++) {
        pid_t pid = fork();
        if (pid == 0) { 
            char cmd[BUFFER_SIZE];
            sprintf(cmd, "%s_part_%d", filename, i);
            execl("./child_process", "child_process", cmd, NULL);
            exit(EXIT_FAILURE); 
        } else if (pid < 0) {
            fprintf(stderr, "Fork failed\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < nChildren; i++) {
        wait(NULL); 
    }

    double totalSum = 0;
    for (int i = 0; i < nChildren; i++) {
        char resultFilename[BUFFER_SIZE];
        sprintf(resultFilename, "%s_part_%d_result", filename, i);

        FILE* resultFile = fopen(resultFilename, "r");
        if (resultFile) {
            double sum = 0;
            if (fscanf(resultFile, "%lf", &sum) == 1) {
                totalSum += sum;
            }
            fclose(resultFile);
        }

        remove(resultFilename);

        char partFilename[BUFFER_SIZE];
        sprintf(partFilename, "%s_part_%d", filename, i);
        remove(partFilename);
    }

    printf("Total sum of squares: %lf\n", totalSum);

    return EXIT_SUCCESS;
}
