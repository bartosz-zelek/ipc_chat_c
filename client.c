#include "headers/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

// login user and save user data in user_arg
int login_procedure(int main_queue_id, User* user_arg){
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

    if (msgsnd(main_queue_id, &user, sizeof(User)-sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
        exit(1);
    }

    LoginResponse response;
    if (msgrcv(main_queue_id, &response, sizeof(LoginResponse)-sizeof(long), getpid(), 0) == -1){
        perror("Error: Could not receive message from server");
        exit(1);
    }
    if (response.success){
        printf("Login successful\n");
        user_arg->pid = user.pid;
        strcpy(user_arg->username, user.username);
        strcpy(user_arg->password, user.password);
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
    if (msgsnd(private_queue_id, &user, sizeof(user)-sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
        exit(1);
    }
    return 1;
}

int check_loggedin_users(int private_queue_id){
    Message msg;
    msg.mtype = PROT_CHECK_LOGGEDIN_REQUEST;
    if (msgsnd(private_queue_id, &msg, sizeof(Message)-sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
        exit(1);
    }
    if (msgrcv(private_queue_id, &msg, sizeof(Message)-sizeof(long), PROT_CHECK_LOGGEDIN_RESPONSE, 0) == -1){
        perror("Error: Could not receive message from server");
        exit(1);
    } else{
        printf("Logged in users:\n%s\n", msg.string);
    }
    return 1;
}

int main(int argc, char* argv[]){
    int main_queue_id = msgget(MAIN_QUEUE_HEX, 0666 | IPC_CREAT);
    if (main_queue_id == -1){
        perror("Error: Could not create main queue");
        exit(1);
    }

    User* user;
    while (login_procedure(main_queue_id, user) == 0); // login loop

    int private_queue_id = msgget(user->pid, 0666 | IPC_CREAT);

    int action;
    while (1){
        printf("1. Logout and exit\n");
        printf("2. Check logged in users\n");
        scanf("%d", &action);
        switch (action){
            case 1:
                logout_procedure(private_queue_id);
                exit(0);
                break;
            case 2:
                check_loggedin_users(private_queue_id);
                break;
            default:
                printf("Invalid action\n");
                break;
        }
    }
    return 0;
}