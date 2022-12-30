#include "headers/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

int login_procedure(int main_queue_id){
    char* username = malloc(sizeof(char) * MAX_USERNAME_LENGTH);
    char* password = malloc(sizeof(char) * MAX_PASSWORD_LENGTH);
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    User user;
    user.mtype = PROT_LOGIN;
    user.pid = getpid();
    strcpy(user.username, username);
    strcpy(user.password, password);

    msgsnd(main_queue_id, &user, sizeof(user)-sizeof(long), 0);

    struct LoginResponse response;
    msgrcv(main_queue_id, &response, sizeof(response)-sizeof(long), getpid(), 0);
    if (response.success){
        printf("Login successful\n");
        return 1;
    } else {
        printf("Login failed\n");
        return 0;
    }
}

int logout_procedure(int private_queue_id){
    User user;
    user.mtype = PROT_LOGOUT;
    user.pid = getpid();
    msgsnd(private_queue_id, &user, sizeof(user)-sizeof(long), 0);
    return 0;
}

int main(int argc, char* argv[]){
    int main_queue_id = msgget(MAIN_QUEUE_ID, 0666 | IPC_CREAT);
    if (main_queue_id == -1){
        perror("Error: Could not create main queue");
        exit(1);
    }
    while (login_procedure(main_queue_id) == 0);
    int private_queue_id = msgget(getpid(), 0666 | IPC_CREAT);
    User user;
    user.pid = getpid();

    int action;
    while (1){
        printf("1. Logout and exit\n");
        scanf("%d", &action);
        switch (action){
            case 1:
                logout_procedure(private_queue_id);
                exit(0);
        }
    }
    return 0;
}