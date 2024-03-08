#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 1024

void distributeNumbers(const char* filename, int nChildren, int count) {
    FILE* mainFile = fopen(filename, "r");
    if (!mainFile) {
        perror("Error opening main file");
        exit(EXIT_FAILURE);
    }

    double numbers[count];
    int index = 0;
    while (fscanf(mainFile, "%lf", &numbers[index]) == 1 && index < count) {
        index++;
    }
    fclose(mainFile);

    for (int i = 0; i < nChildren; i++) {
        char childFilename[BUFFER_SIZE];
        snprintf(childFilename, BUFFER_SIZE, "%s_%d", filename, i);
        
        FILE* childFile = fopen(childFilename, "w");
        if (!childFile) {
            perror("Error creating child input file");
            exit(EXIT_FAILURE);
        }
        
        int start = (count / nChildren) * i;
        int end = (i == nChildren - 1) ? count : (count / nChildren) * (i + 1);
        for (int j = start; j < end; j++) {
            fprintf(childFile, "%lf\n", numbers[j]);
        }
        fclose(childFile);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <number_of_children>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];
    int nChildren = atoi(argv[2]);
    int count = 0;

    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    double number;
    while (fscanf(file, "%lf", &number) == 1) {
        count++;
    }
    fclose(file);

    if (nChildren > count) {
        nChildren = count;
        printf("Adjusting number of children to %d due to count of numbers.\n", nChildren);
    }

    distributeNumbers(filename, nChildren, count);

    for (int i = 0; i < nChildren; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            char childFilename[BUFFER_SIZE];
            snprintf(childFilename, BUFFER_SIZE, "%s_%d", filename, i);
            execl("./child_process", "child_process", childFilename, NULL);
            perror("Error in execl");
            exit(EXIT_FAILURE);
        }
    }

    printf("Parent process is sleeping, allowing zombie processes to appear...\n");
    sleep(30);

    double totalSum = 0;
    for (int i = 0; i < nChildren; ++i) {
        char tempFilename[BUFFER_SIZE];
        snprintf(tempFilename, BUFFER_SIZE, "%s_%d", filename, i);
        remove(tempFilename);
        
        char resultFilename[BUFFER_SIZE];
        snprintf(resultFilename, BUFFER_SIZE, "%s_%d_result", filename, i);
        FILE* resultFile = fopen(resultFilename, "r");
        if (resultFile) {
            double sum = 0;
            if (fscanf(resultFile, "%lf", &sum) == 1) {
                totalSum += sum;
            }
            fclose(resultFile);
            remove(resultFilename);
        }
    }

    printf("Total sum of squares: %lf\n", totalSum);

    sleep(10);

    return EXIT_SUCCESS;
}
