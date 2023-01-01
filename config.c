#include "headers/config.h"

#define MAX_LINE_LENGTH 200
#define MAX_USERNAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20

#define MAX_LINE_LENGTH 200
#define MAX_RECORDS 100

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* open_config_file(const char* filename){
    FILE* fp = fopen(filename, "r");
    if(fp == NULL){
        perror("Error: Could not open config file");
        exit(1);
    }
    return fp;
}

char** read_section(const char* filename, const char* section){
    FILE* fp = open_config_file(filename);
    char** records = malloc(sizeof(char*) * MAX_RECORDS+1);
    char* line = malloc(sizeof(char) * MAX_LINE_LENGTH);

    int boolSpecificSection = 0;
    int recordIndex = 0;

    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL){
        if (line[0] == '#'){ // Comment
            continue;
        }
        if (strchr(line, ':') != NULL){ // Some section header; char ':' can be used only in section headers
            if (boolSpecificSection == 1){ // We are done reading section
                break;
            }
            if (boolSpecificSection == 0 && strncmp(line, section, strlen(section)-1) == 0){ // We can start reading users; ignore new line character during comparison
                boolSpecificSection = 1;
                continue;
            }
        }
        if (boolSpecificSection == 1){ // Start reading users
            records[recordIndex] = malloc(sizeof(char) * MAX_LINE_LENGTH);
            strcpy(records[recordIndex], line);
            records[recordIndex][strlen(line)-1] = '\0'; // Remove new line character
            recordIndex++;
        }
    }
    fclose(fp);
    free(line);
    records[recordIndex] = NULL; // NULL at the end of array
    return records;
}