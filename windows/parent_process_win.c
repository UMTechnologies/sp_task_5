#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

#define BUFFER_SIZE 1024

int checkFileContent(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filePath);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize == 0) {
        fprintf(stderr, "Error: File %s is empty.\n", filePath);
        fclose(file);
        return -2;
    }
    rewind(file);

    double number;
    int validNumbers = 0;
    while (fscanf(file, "%lf", &number) == 1) {
        validNumbers++;
    }

    fclose(file);

    if (validNumbers == 0) {
        fprintf(stderr, "Error: No valid numbers found in file %s.\n", filePath);
        return -3;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_children> <path_to_numbers_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fileCheckResult = checkFileContent(argv[2]);
    if (fileCheckResult != 0) {
        exit(EXIT_FAILURE);
    }

    int nChildren = atoi(argv[1]);
    if (nChildren <= 0) {
        fprintf(stderr, "Error: The number of children must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    int totalNumbers = 10;

    STARTUPINFO si[nChildren];
    PROCESS_INFORMATION pi[nChildren];
    char cmdLine[BUFFER_SIZE];

    for (int i = 0; i < nChildren; i++) {
        ZeroMemory(&si[i], sizeof(si[i]));
        si[i].cb = sizeof(si[i]);
        ZeroMemory(&pi[i], sizeof(pi[i]));

        sprintf(cmdLine, "child_process_win.exe %d %d %d \"%s\"", i, totalNumbers, i, argv[2]);

        if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si[i], &pi[i])) {
            fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
            exit(EXIT_FAILURE);
        }
    }

    double totalSum = 0;
    for (int i = 0; i < nChildren; i++) {
        WaitForSingleObject(pi[i].hProcess, INFINITE);
        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);

        char resultFilename[BUFFER_SIZE];
        sprintf(resultFilename, "result_%d.txt", i);
        FILE* resultFile = fopen(resultFilename, "r");
        if (!resultFile) {
            fprintf(stderr, "Error opening result file: %s\n", resultFilename);
            continue;
        }

        double sum;
        if (fscanf(resultFile, "%lf", &sum) == 1) {
            totalSum += sum;
        }
        fclose(resultFile);
        remove(resultFilename);
    }

    printf("Total sum of squares: %f\n", totalSum);

    return 0;
}
