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
    // char* username = malloc(sizeof(char) * MAX_USERNAME_LENGTH);
    // char* password = malloc(sizeof(char) * MAX_PASSWORD_LENGTH);
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
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
        // exit(1);
    }

    SuccessResponse response;
    if (msgrcv(main_queue_id, &response, sizeof(SuccessResponse)-sizeof(long), getpid(), 0) == -1){
        perror("Error: Could not receive message from server");
        // exit(1);
    }
    if (response.success){
        printf("Login successful\n\n");
        user_arg->pid = user.pid;
        strcpy(user_arg->username, user.username);
        strcpy(user_arg->password, user.password);
        return 1;
    } else {
        printf("Login failed\n\n");
        return 0;
    }
}

void logout_procedure(int private_queue_id){
    User user;
    user.mtype = PROT_LOGOUT;
    user.pid = getpid();
    if (msgsnd(private_queue_id, &user, sizeof(user)-sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
        // exit(1);
    }
}

char* check_loggedin_users(int private_queue_id){
    Message msg;
    msg.mtype = PROT_CHECK_LOGGEDIN_REQUEST;
    if (msgsnd(private_queue_id, &msg, sizeof(Message)-sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
        // exit(1);
    }
    if (msgrcv(private_queue_id, &msg, sizeof(Message)-sizeof(long), PROT_CHECK_LOGGEDIN_RESPONSE, 0) == -1){
        perror("Error: Could not receive message from server");
        // exit(1);
    }
    // else{
    //     printf("Logged in users:\n%s\n", msg.string);
    // }
    char* users = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    strcpy(users, msg.string);
    return users;
}

char* check_groups(int private_queue_id){
    Message msg;
    msg.mtype = PROT_CHECK_GROUPS_REQUEST;
    if (msgsnd(private_queue_id, &msg, sizeof(Message)-sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
        // exit(1);
    }
    if (msgrcv(private_queue_id, &msg, sizeof(Message)-sizeof(long), PROT_CHECK_GROUPS_RESPONSE, 0) == -1){
        perror("Error: Could not receive message from server");
        // exit(1);
    }
    // else{
    //     printf("Groups:\n%s\n", msg.string);
    // }
    char* groups = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    strcpy(groups, msg.string);
    return groups;
}

char* check_users_in_group(int private_queue_id){
    char group_name[MAX_GROUP_NAME_LENGTH];
    printf("Enter group name: ");
    scanf("%s", group_name);
    Message msg;
    msg.mtype = PROT_CHECK_USERS_IN_GROUP_REQUEST;
    strcpy(msg.string, group_name);
    if (msgsnd(private_queue_id, &msg, sizeof(Message)-sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
        // exit(1);
    }
    if (msgrcv(private_queue_id, &msg, sizeof(Message)-sizeof(long), PROT_CHECK_USERS_IN_GROUP_RESPONSE, 0) == -1){
        perror("Error: Could not receive message from server");
        // exit(1);
    }
    // else{
    //     printf("Users in group %s:\n%s\n", group_name, msg.string);
    // }
    char* users = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    strcpy(users, msg.string);
    return users;
}

void enroll_to_group(int private_queue_id){
    char group_name[MAX_GROUP_NAME_LENGTH];
    printf("Enter group name: ");
    scanf("%s", group_name);
    Message msg;
    msg.mtype = PROT_ENROLL_TO_GROUP_REQUEST;
    strcpy(msg.string, group_name);
    if (msgsnd(private_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
    }
    if (msgrcv(private_queue_id, &msg, sizeof(Message) - sizeof(long), PROT_ENROLL_TO_GROUP_RESPONSE, 0) == -1){
        perror("Error: Could not recieve message from server");
    }else{
        printf("\n%s\n", msg.string);
    }
}

void unenroll_from_group(int private_queue_id){
    char group_name[MAX_GROUP_NAME_LENGTH];
    printf("Enter group name: ");
    scanf("%s", group_name);
    Message msg;
    msg.mtype = PROT_UNENROLL_FROM_GROUP_REQUEST;
    strcpy(msg.string, group_name);
    if (msgsnd(private_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
    }
    if (msgrcv(private_queue_id, &msg, sizeof(Message) - sizeof(long), PROT_UNENROLL_FROM_GROUP_RESPONSE, 0) == -1){
        perror("Error: Could not recieve message from server");
    } else{
        printf("\n%s\n", msg.string);
    }
}

void set_message_to_user(char *user, int private_queue_id)
{
    printf("Enter content of the message:\n");
    char mess[MAX_MESSAGE_LENGTH];
    scanf("%s", mess);
    // printf("Enter user to which you want to send message to:\n");
    // char user[MAX_USERNAME_LENGTH];
    // scanf("%s", user);
    Message msg;
    msg.mtype = PROT_SEND_MESSAGE_TO_USER;
    strcpy(msg.string, mess);
    if (msgsnd(private_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1){
        perror("Error: Could not send message to server");
    }
    if (msgrcv(private_queue_id, &msg, sizeof(Message) - sizeof(long), PROT_SEND_MESSAGE_TO_USER_RESPONSE,  0) == -1){
        perror("Error: Could not recieve message from server");
    } else{
        printf("\n%s\n", msg.string);
    }
}

int main(int argc, char* argv[]){
    int main_queue_id = msgget(MAIN_QUEUE_HEX, 0666 | IPC_CREAT);
    if (main_queue_id == -1){
        perror("Error: Could not create main queue");
        exit(1);
    }

    User* user = malloc(sizeof(User));
    while (login_procedure(main_queue_id, user) == 0); // login loop

    int private_queue_id = msgget(user->pid, 0666 | IPC_CREAT);

    int action;
    while (1){
        printf("1. Logout and exit\n");
        printf("2. Check logged in users\n");
        printf("3. Check groups\n");
        printf("4. Check users in group\n");
        printf("5. Enroll to group\n");
        printf("6. Unenroll from group\n");
        printf("Enter action: ");
        scanf("%d", &action);
        printf("\n");
        switch (action){
            case 1:
                logout_procedure(private_queue_id);
                exit(0);
                break;
            case 2:
                printf("Logged in users:\n%s\n", check_loggedin_users(private_queue_id));
                break;
            case 3:
                printf("Groups:\n%s\n", check_groups(private_queue_id));
                break;
            case 4:
                printf("Users in group:\n%s\n", check_users_in_group(private_queue_id));
                break;
            case 5:
                enroll_to_group(private_queue_id);
                break;
            case 6:
                unenroll_from_group(private_queue_id);
                break;
            default:
                printf("Invalid action\n");
                break;
        }
    }
    return 0;
}