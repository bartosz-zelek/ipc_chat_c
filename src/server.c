#include "../headers/config.h"
#include "../headers/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

typedef struct{
    char* username;
    int pid;
    int queue_id;
} UserData;

// returns something like [ [user1, password1], [user2, password2], ... , NULL]
char*** make_pairs_user_password(){
    char** users_with_passwords = read_section("config.txt", "USERS:");
    char*** pairs_user_password = malloc(sizeof(char**) * MAX_USERS+1);
    int i;
    for (i=0; users_with_passwords[i] != NULL; i++){
        char* user = strtok(users_with_passwords[i], " ");
        char* password = strtok(NULL, " ");
        pairs_user_password[i] = malloc(sizeof(char*) * 2);
        pairs_user_password[i][0] = malloc(sizeof(user));
        pairs_user_password[i][1] = malloc(sizeof(user));
        strcpy(pairs_user_password[i][0], user);
        strcpy(pairs_user_password[i][1], password);
    }
    pairs_user_password[i] = NULL;
    free(users_with_passwords);
    return pairs_user_password;
}

int are_credentials_valid(char* username, char* password){
    char*** pairs_user_password = make_pairs_user_password();
    for (int i=0; pairs_user_password[i] != NULL; i++){
        if (strcmp(pairs_user_password[i][0], username) == 0 && strcmp(pairs_user_password[i][1], password) == 0){
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]){
    int main_queue_id = msgget(0x2137, 0666 | IPC_CREAT);
    if (main_queue_id == -1){
        perror("Error: Could not create main queue");
        exit(1);
    }

    struct User user;
    UserData* logged_users[MAX_USERS] = {NULL};

    while (1){
        if (msgrcv(main_queue_id, &user, sizeof(user), PROT_LOGIN, IPC_NOWAIT) != -1){ // check if someone is trying to log in
            if (are_credentials_valid(user.username, user.password)){
                // user is valid
                // create new queue for this user
                // mtype = user.pid
                // new queue id = user.pid

                printf("User %s logged in\n", user.username);
                int server_process_queue = msgget(user.pid, 0666 | IPC_CREAT);
                if (server_process_queue == -1){
                    perror("Error: Could not create new queue");
                    exit(1);
                }

                struct LoginResponse response;
                response.mtype = user.pid;
                response.success = 1;

                msgsnd(main_queue_id, &response, sizeof(response), 0);

                // add user to pairs_user_queueid
                int i;
                for (i=0; logged_users[i] != NULL; i++);
                logged_users[i] = (UserData*)malloc(sizeof(UserData));
                logged_users[i]->username = malloc(sizeof(user.username));
                strcpy(logged_users[i]->username, user.username);
                logged_users[i]->pid = user.pid;
                logged_users[i]->queue_id = server_process_queue;


            } else {
                struct LoginResponse response;
                response.mtype = user.pid;
                response.success = 0;
                msgsnd(main_queue_id, &response, sizeof(response), 0);
                printf("User %s failed to log in\n", user.username);
            }
        }

        for (int i=0; i<MAX_USERS; i++){ // check if someone is trying to log out
            if (logged_users[i] != NULL && msgrcv(logged_users[i]->queue_id, &user, sizeof(user), PROT_LOGOUT, IPC_NOWAIT) != -1){
                printf("User %s logged out\n", logged_users[i]->username); // leak
                // msgctl(logged_users[i]->queue_id, IPC_RMID, NULL);
                // free(logged_users[i]);
                // logged_users[i] = NULL;
            }
        }

    }

    return 0;
}