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

typedef struct{
    char name[MAX_GROUP_NAME_LENGTH];
    char* usernames[MAX_USERS+1];
} GroupData;

typedef struct{
    char username[MAX_USERNAME_LENGTH];
    int attempts;
} UnsuccessfulLoginAttempt;



// returns array with UserData* elements, where only username and password are set
UserData** read_users_data_from_config(){
    char** users_with_passwords = read_section(CONFIG_FILE, "USERS:");
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

// array of GroupData* elements, where name and usernames are set
GroupData** read_groups_data_from_config(){
    char** groups_users = read_section(CONFIG_FILE, "GROUPS:");
    GroupData** groups_data = malloc(sizeof(GroupData*) * MAX_GROUPS+1);
    int i;
    for (i=0; groups_users[i] != NULL; i++){
        char* group_name = strtok(groups_users[i], " ");

        groups_data[i] = malloc(sizeof(GroupData));
        memset(groups_data[i]->usernames, 0, sizeof(groups_data[i]->usernames));

        strcpy(groups_data[i]->name, group_name);
        int j = 0;
        while (group_name != NULL){
            char* username = strtok(NULL, " ");
            if (username == NULL){
                break;
            }
            groups_data[i]->usernames[j] = malloc(sizeof(char) * MAX_USERNAME_LENGTH);
            strcpy(groups_data[i]->usernames[j], username);
            j++;
        }
        groups_data[i]->usernames[j] = NULL;
    }
    groups_data[i] = NULL;
    for (int i=0; groups_users[i] != NULL; i++){
        free(groups_users[i]);
        groups_users[i] = NULL;
    }
    free(groups_users);
    return groups_data;
}

int is_user_blocked(char* username, UnsuccessfulLoginAttempt** unsuccessful_login_attempts){
    for (int i=0; unsuccessful_login_attempts[i] != NULL && i<MAX_USERS; i++){
        if (strcmp(unsuccessful_login_attempts[i]->username, username) == 0){
            if (unsuccessful_login_attempts[i]->attempts >= MAX_UNSUCCESSFUL_LOGIN_ATTEMPTS){
                return 1;
            }
        }
    }
    return 0;
}

void reset_unsuccessful_login_attempts(char* username, UnsuccessfulLoginAttempt** unsuccessful_login_attempts){
    for (int i=0; unsuccessful_login_attempts[i] != NULL && i<MAX_USERS; i++){
        if (strcmp(unsuccessful_login_attempts[i]->username, username) == 0){
            unsuccessful_login_attempts[i]->attempts = 0;
        }
    }
}

int is_user_loggedin(char* username, UserData** logged_users){
    for (int i=0; logged_users[i] != NULL && i<MAX_USERS; i++){
        if (strcmp(logged_users[i]->username, username) == 0){
            return 1;
        }
    }
    return 0;
}

void catch_and_perform_login_action(int main_queue_id, UserData** users_data, UserData** logged_users, UnsuccessfulLoginAttempt** unsuccessful_login_attempts){
    User tmp_user;

    if (msgrcv(main_queue_id, &tmp_user, sizeof(User)-sizeof(long), PROT_LOGIN, IPC_NOWAIT) != -1){ // check if someone is trying to log in (public queue)
        if (are_credentials_valid(tmp_user.username, tmp_user.password, users_data) && !is_user_blocked(tmp_user.username, unsuccessful_login_attempts) && !is_user_loggedin(tmp_user.username, logged_users)){
            // user is valid
            // create new queue for this user
            // new queue id = tmp_user.pid

            reset_unsuccessful_login_attempts(tmp_user.username, unsuccessful_login_attempts);

            printf("User %s logged in\n", tmp_user.username);
            int server_process_queue = msgget(tmp_user.pid, 0666 | IPC_CREAT);
            printf("Kolejka prywatna: %d dla usera: %d\n", server_process_queue, tmp_user.pid);
            if (server_process_queue == -1){
                perror("Error: Could not create new queue");
                // exit(1);
            }

            SuccessResponse response;
            response.mtype = tmp_user.pid;
            response.success = 1;

            if (msgsnd(main_queue_id, &response, sizeof(SuccessResponse)-sizeof(long), 0) == -1){
                perror("Error: Could not send response to client");
                // exit(1);
            }

            // add user to pairs_user_queueid
            int i;
            for (i=0; logged_users[i] != NULL; i++);
            logged_users[i] = (UserData*)malloc(sizeof(UserData));
            strcpy(logged_users[i]->username, tmp_user.username);
            logged_users[i]->pid = tmp_user.pid;
            logged_users[i]->queue_id = server_process_queue;


        } else {
            SuccessResponse response;
            response.mtype = tmp_user.pid;
            response.success = 0;

            if (is_user_loggedin(tmp_user.username, logged_users)){
                strcpy(response.string, "User already logged in");
            } else{
                // -- CHECK UNSUCCESSFUL LOGIN ATTEMPTS --
                memset(response.string, 0, sizeof(response.string));
                int i;
                int bool_found_username = 0;
                for (i=0; unsuccessful_login_attempts[i] != NULL && i<MAX_USERS; i++){
                    if (strcmp(unsuccessful_login_attempts[i]->username, tmp_user.username) == 0){
                        bool_found_username = 1;
                        unsuccessful_login_attempts[i]->attempts++;
                        if (unsuccessful_login_attempts[i]->attempts >= MAX_UNSUCCESSFUL_LOGIN_ATTEMPTS){
                            printf("User %s blocked\n", tmp_user.username);
                            response.success = 0;
                            strcpy(response.string, "User blocked (too many unsuccessful login attempts - contact with administrator)" );
                        }
                        break;
                    }
                }
                if (bool_found_username == 0){
                    for (i=0; unsuccessful_login_attempts[i] != NULL && i<MAX_USERS; i++);
                    unsuccessful_login_attempts[i] = (UnsuccessfulLoginAttempt*)malloc(sizeof(UnsuccessfulLoginAttempt));
                    strcpy(unsuccessful_login_attempts[i]->username, tmp_user.username);
                    unsuccessful_login_attempts[i]->attempts = 1;
                }
                if (strlen(response.string) == 0){ // it is not the best solution
                    char msg[MAX_MESSAGE_LENGTH];

                    snprintf(msg, sizeof(msg), "Unsuccessful attempts for user %s (%d/%d)", tmp_user.username, unsuccessful_login_attempts[i]->attempts, MAX_UNSUCCESSFUL_LOGIN_ATTEMPTS);
                    strcpy(response.string, msg);
                }
                // -- CHECK UNSUCCESSFUL LOGIN ATTEMPTS --
            }

            if (msgsnd(main_queue_id, &response, sizeof(SuccessResponse)-sizeof(long), 0) == -1){
                perror("Error: Could not send response to client");
                // exit(1);
            }
            printf("User %s failed to log in\n", tmp_user.username);
        }
    }
}

void catch_and_perform_logout_action(UserData** logged_users){
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

void catch_and_perform_check_loggedin_users_action(UserData** logged_users){
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
                    // exit(1);
                }
            }
        }
    }
}

void catch_and_perform_check_groups_action(GroupData** groups_data, UserData** logged_users){
    Message msg;
    for (int i=0; i<MAX_USERS; i++){
        if (logged_users[i] != NULL){
            if (msgrcv(logged_users[i]->queue_id, &msg, sizeof(Message)-sizeof(long), PROT_CHECK_GROUPS_REQUEST, IPC_NOWAIT) != -1){
                memset(msg.string, 0, sizeof(msg.string));

                for (int j=0; j<MAX_GROUPS; j++){
                    if (groups_data[j] != NULL){
                        strcat(msg.string, groups_data[j]->name);
                        strcat(msg.string, "\n");
                    }
                }
                printf("Sending list of groups to user %s\n", logged_users[i]->username);
                msg.mtype = PROT_CHECK_GROUPS_RESPONSE;
                if (msgsnd(logged_users[i]->queue_id, &msg, sizeof(Message)-sizeof(long), 0) == -1){
                    perror("Error: Could not send response to client");
                    // exit(1);
                }
            }
        }
    }
}

void catch_and_perform_check_users_in_group_action(GroupData** groups_data, UserData** logged_users){
    Message msg;
    char group_name[MAX_GROUP_NAME_LENGTH];
    for (int i=0; i<MAX_USERS; i++){
        if (logged_users[i] != NULL){
            if (msgrcv(logged_users[i]->queue_id, &msg, sizeof(Message)-sizeof(long), PROT_CHECK_USERS_IN_GROUP_REQUEST, IPC_NOWAIT) != -1){
                strcpy(group_name, msg.string);
                memset(msg.string, 0, sizeof(msg.string));

                int j;
                for (j=0; j<MAX_GROUPS; j++){
                    if (groups_data[j] != NULL){
                        if (strcmp(groups_data[j]->name, group_name) == 0){
                            for (int k=0; k<MAX_USERS; k++){
                                if (groups_data[j]->usernames[k] != NULL){
                                    strcat(msg.string, groups_data[j]->usernames[k]);
                                    strcat(msg.string, "\n");
                                }
                            }
                            break;
                        }
                    }
                }

                if (j == MAX_GROUPS){
                    printf("Group %s does not exist\n", group_name);
                    strcat(msg.string, "This group does not exist\n");
                } else if(msg.string[0] == '\0'){
                    printf("Group %s is empty\n", group_name);
                    strcat(msg.string, "This group is empty\n");
                }

                printf("Sending list of group members to user %s\n", logged_users[i]->username);
                msg.mtype = PROT_CHECK_USERS_IN_GROUP_RESPONSE;
                if (msgsnd(logged_users[i]->queue_id, &msg, sizeof(Message)-sizeof(long), 0) == -1){
                    perror("Error: Could not send response to client");
                    // exit(1);
                }
            }
        }
    }
}

void catch_and_perform_enroll_to_group_action(GroupData** groups_data, UserData** logged_users){
    Message msg;
    char group_name[MAX_GROUP_NAME_LENGTH];
    for (int i=0; i<MAX_USERS; i++){
        if (logged_users[i] != NULL){
            if (msgrcv(logged_users[i]->queue_id, &msg, sizeof(Message)-sizeof(long), PROT_ENROLL_TO_GROUP_REQUEST, IPC_NOWAIT) != -1){
                strcpy(group_name, msg.string);
                memset(msg.string, 0, sizeof(msg.string));

                int j, l=-1; // l is index for new user to be save; j is for choosed group; i is for user who sent request
                for (j=0; j<MAX_GROUPS; j++){
                    if (groups_data[j] != NULL){
                        if (strcmp(groups_data[j]->name, group_name) == 0){
                            for (int k=0; k<MAX_USERS; k++){
                                if (groups_data[j]->usernames[k] != NULL && strcmp(logged_users[i]->username, groups_data[j]->usernames[k]) == 0) {// check if user is already enrolled in this group
                                    printf("User is already enrolled in this group.\n");
                                    l=-1;
                                    break;
                                }
                                if (groups_data[j]->usernames[k] == NULL && l == -1){ // found place to save user, but don't break - check if already enrolled
                                    l = k;
                                }
                            }
                            break;
                        }
                    }
                }

                if (l != -1){ // found place to save user's username, and user is not already enrolled in group
                    groups_data[j]->usernames[l] = malloc(sizeof(char)*MAX_USERNAME_LENGTH);
                    strcpy(groups_data[j]->usernames[l], logged_users[i]->username);
                    strcpy(msg.string, "Succesfully enrolled user to the group.\n");
                } else if (j != MAX_GROUPS){ // found such group, but user is enrolled
                    strcpy(msg.string, "User is already enrolled in this group.\n");
                } else{ // not found such group
                    strcpy(msg.string, "This group does not exist\n");
                }

                msg.mtype = PROT_ENROLL_TO_GROUP_RESPONSE;
                if (msgsnd(logged_users[i]->queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1){
                    perror("Error: Could not send response to client");
                }
            }
        }
    }
}

void catch_and_perform_unenroll_from_group_action(GroupData** groups_data, UserData** logged_users){
    Message msg;
    char group_name[MAX_GROUP_NAME_LENGTH];
    for (int i=0; i<MAX_USERS; i++){
        if (logged_users[i] != NULL){
            if (msgrcv(logged_users[i]->queue_id, &msg, sizeof(Message)-sizeof(long), PROT_UNENROLL_FROM_GROUP_REQUEST, IPC_NOWAIT) != -1){
                strcpy(group_name, msg.string);
                memset(msg.string, 0, sizeof(msg.string));

                int k, j; // k is index of user's username, if user is not in this group k=MAX_USERS; j is index of group, j=MAX_GROUPS if not found
                for (j=0; j<MAX_GROUPS; j++){
                    if (groups_data[j] != NULL){
                        if (strcmp(groups_data[j]->name, group_name) == 0){
                            for (k=0; k<MAX_USERS; k++){
                                if (groups_data[j]->usernames[k] != NULL && strcmp(groups_data[j]->usernames[k], logged_users[i]->username) == 0){
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }

                if (j != MAX_GROUPS){ // found such group
                    if (k != MAX_USERS){ // user has been found in this group
                        free(groups_data[j]->usernames[k]);
                        groups_data[j]->usernames[k] = NULL;
                        strcpy(msg.string, "User has been unenrolled from the group.\n");
                        printf("User has been unenrolled from the group.\n");
                    } else{ // user hasn't been found in this group
                        printf("User is absent in this group.\n");
                        strcpy(msg.string, "User is absent in this group.\n");
                    }
                } else{ // not found such group
                    printf("This group does not exist\n");
                    strcpy(msg.string, "This group does not exist\n");
                }

                msg.mtype = PROT_UNENROLL_FROM_GROUP_RESPONSE;
                if (msgsnd(logged_users[i]->queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1){
                    perror("Error: Could not send response to client");
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
    UserData** users_from_config = read_users_data_from_config(); // array of users from config file - size is MAX_USERS+1 (last element is NULL)
    GroupData** groups_data = read_groups_data_from_config(); // array of groups from config file - size is MAX_GROUPS+1 (last element is NULL)
    UnsuccessfulLoginAttempt** unsuccessful_login_attempts = malloc(sizeof(UnsuccessfulLoginAttempt*)*MAX_USERS); // array of unsuccessful login attempts - size is MAX_USERS

    for (int i=0; i<MAX_USERS; i++){
        unsuccessful_login_attempts[i] = NULL;
    }

    while (1){
        catch_and_perform_login_action(main_queue_id, users_from_config, logged_users, unsuccessful_login_attempts);

        catch_and_perform_logout_action(logged_users);

        catch_and_perform_check_loggedin_users_action(logged_users);

        catch_and_perform_check_groups_action(groups_data, logged_users);

        catch_and_perform_check_users_in_group_action(groups_data, logged_users);

        catch_and_perform_enroll_to_group_action(groups_data, logged_users);

        catch_and_perform_unenroll_from_group_action(groups_data, logged_users);
    }

    return 0;
}