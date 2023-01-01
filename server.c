#include "headers/config.h"
#include "headers/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

//struct User is defined in common.h and is used to send messages between server and client

typedef struct{ // similar to User, but for inner use
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int pid;
    int queue_id;
} UserData;

// returns array with UserData* elements, where only username and password are set
UserData** read_users_data_from_config(char* config_file){
    char** users_with_passwords = read_section(config_file, "USERS:");
    UserData** users_data = malloc(sizeof(UserData*) * MAX_USERS+1);
    int i;
    for (i=0; users_with_passwords[i] != NULL; i++){
        char* user = strtok(users_with_passwords[i], " ");
        char* password = strtok(NULL, " ");
        users_data[i] = malloc(sizeof(UserData));
        strcpy(users_data[i]->username, user);
        strcpy(users_data[i]->password, password);
    }
    users_data[i] = NULL;
    for (int i=0; users_with_passwords[i] != NULL; i++){
        free(users_with_passwords[i]);
        users_with_passwords[i] = NULL;
    }
    free(users_with_passwords);
    return users_data;
}

int are_credentials_valid(char* username, char* password, UserData** users_data){
    for (int i=0; users_data[i] != NULL; i++){
        if (strcmp(users_data[i]->username, username) == 0 && strcmp(users_data[i]->password, password) == 0){
            return 1;
        }
    }
    return 0;
}

void catch_and_perform_login_action(int main_queue_id, UserData** users_data, UserData** logged_users){
    // TODO: don't let log in if user is already logged in (same username)
    // TODO: take into account how many times user tried to log in (if more than X times, then block user [contact admin])
    User tmp_user;

    if (msgrcv(main_queue_id, &tmp_user, sizeof(User)-sizeof(long), PROT_LOGIN, IPC_NOWAIT) != -1){ // check if someone is trying to log in (public queue)
        if (are_credentials_valid(tmp_user.username, tmp_user.password, users_data)){
            // user is valid
            // create new queue for this user
            // new queue id = tmp_user.pid

            printf("User %s logged in\n", tmp_user.username);
            int server_process_queue = msgget(tmp_user.pid, 0666 | IPC_CREAT);
            printf("Kolejka prywatna: %d dla usera: %d\n", server_process_queue, tmp_user.pid);
            if (server_process_queue == -1){
                perror("Error: Could not create new queue");
                exit(1);
            }

            LoginResponse response;
            response.mtype = tmp_user.pid;
            response.success = 1;

            if (msgsnd(main_queue_id, &response, sizeof(LoginResponse)-sizeof(long), 0) == -1){
                perror("Error: Could not send response to client");
                exit(1);
            }

            // add user to pairs_user_queueid
            int i;
            for (i=0; logged_users[i] != NULL; i++);
            logged_users[i] = (UserData*)malloc(sizeof(UserData));
            strcpy(logged_users[i]->username, tmp_user.username);
            logged_users[i]->pid = tmp_user.pid;
            logged_users[i]->queue_id = server_process_queue;


        } else {
            LoginResponse response;
            response.mtype = tmp_user.pid;
            response.success = 0;
            if (msgsnd(main_queue_id, &response, sizeof(LoginResponse)-sizeof(long), 0) == -1){
                perror("Error: Could not send response to client");
                exit(1);
            }
            printf("User %s failed to log in\n", tmp_user.username);
        }
    }
}

void catch_and_perform_logout_action(int main_queue_id, UserData** users_data, UserData** logged_users){
    User tmp_user;

    for (int i=0; i<MAX_USERS; i++){ // check if someone is trying to log out
        if (logged_users[i] != NULL){
            if (msgrcv(logged_users[i]->queue_id, &tmp_user, sizeof(User)-sizeof(long), PROT_LOGOUT, IPC_NOWAIT) != -1){ // check if someone is trying to log out (private queue)
                printf("User %s logged out\n", logged_users[i]->username);
                msgctl(logged_users[i]->queue_id, IPC_RMID, NULL);
                free(logged_users[i]);
                logged_users[i] = NULL;
            }
        }
    }
}

void catch_and_perform_check_loggedin_users_action(int main_queue_id, UserData** users_data, UserData** logged_users){
    Message msg;

    for (int i=0; i<MAX_USERS; i++){
        if (logged_users[i] != NULL){
            if (msgrcv(logged_users[i]->queue_id, &msg, sizeof(Message)-sizeof(long), PROT_CHECK_LOGGEDIN_REQUEST, IPC_NOWAIT) != -1){ // someone requested for list of logged users
                // prepare message
                memset(msg.string, 0, sizeof(msg.string));
                for (int j=0; j<MAX_USERS; j++){
                    if (logged_users[j] != NULL){
                        strcat(msg.string, logged_users[j]->username);
                        strcat(msg.string, "\n");
                    }
                }
                printf("Sending list of logged users to user %s\n", logged_users[i]->username);
                msg.mtype = PROT_CHECK_LOGGEDIN_RESPONSE;
                if (msgsnd(logged_users[i]->queue_id, &msg, sizeof(Message)-sizeof(long), 0) == -1){
                    perror("Error: Could not send response to client");
                    exit(1);
                }
            }
        }
    }
}

int main(int argc, char* argv[]){
    int main_queue_id = msgget(MAIN_QUEUE_HEX, 0666 | IPC_CREAT);
    printf("Kolejka publiczna: %d\n", main_queue_id);
    if (main_queue_id == -1){
        perror("Error: Could not create main queue");
        exit(1);
    }

    UserData* logged_users[MAX_USERS] = {NULL}; // array of logged users - no one is logged in at the beginning
    UserData** users_from_config = read_users_data_from_config("config.txt"); // array of users from config file - size is MAX_USERS+1 (last element is NULL)

    while (1){
        catch_and_perform_login_action(main_queue_id, users_from_config, logged_users);

        catch_and_perform_logout_action(main_queue_id, users_from_config, logged_users);

        catch_and_perform_check_loggedin_users_action(main_queue_id, users_from_config, logged_users);
    }

    return 0;
}