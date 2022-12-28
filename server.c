#include "config.h"

#include <stdio.h>

int main(int argc, char* argv[]){
    char** users = read_section("config.txt", "USERS:");
    for (int i = 0; users[i] != NULL; i++){
        printf("%s\n", users[i]);
    }
    return 0;
}