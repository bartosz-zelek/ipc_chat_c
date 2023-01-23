#pragma once

#define MAIN_QUEUE_HEX 0x2137
#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32
#define MAX_MESSAGE_LENGTH 1000
#define MAX_GROUP_NAME_LENGTH 32
#define MAX_GROUPS 10
#define MAX_LINE_LENGTH 500
#define MAX_USERS 100

#define PROT_LOGIN 1
#define PROT_LOGOUT 2
#define PROT_CHECK_LOGGEDIN_REQUEST 3
#define PROT_CHECK_LOGGEDIN_RESPONSE 4
#define PROT_CHECK_GROUPS_REQUEST 5
#define PROT_CHECK_GROUPS_RESPONSE 6
#define PROT_CHECK_USERS_IN_GROUP_REQUEST 7
#define PROT_CHECK_USERS_IN_GROUP_RESPONSE 8
#define PROT_ENROLL_TO_GROUP_REQUEST 9
#define PROT_ENROLL_TO_GROUP_RESPONSE 10
#define PROT_UNENROLL_FROM_GROUP_REQUEST 11
#define PROT_UNENROLL_FROM_GROUP_RESPONSE 12
#define PROT_SEND_MESSAGE_TO_USER_FROM 13
#define PROT_SEND_MESSAGE_TO_USER_TO 14
#define PROT_SEND_MESSAGE_TO_USER_RESPONSE 15
#define PROT_SEND_MESSAGE_TO_GROUP 16
#define PROT_SEND_MESSAGE_TO_GROUP_RESPONSE 17
#define PROT_BLOCK_USER 18
#define PROT_BLOCK_USER_RESPONSE 19
#define PROT_BLOCK_GROUP 20
#define PROT_BLOCK_GROUP_RESPONSE 21
#define PROT_USER_DOES_NOT_EXIST 22

#define MAX_UNSUCCESSFUL_LOGIN_ATTEMPTS 3 // it must be <=9

#define CONFIG_FILE "config.txt"

typedef struct {
    long mtype;
    int pid;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    // char blocked_groups [MAX_GROUPS][MAX_GROUP_NAME_LENGTH];
    // char blocked_users [MAX_USERS][MAX_USERNAME_LENGTH];
} User;

typedef struct{
    long mtype;
    char string[MAX_MESSAGE_LENGTH];
} Message;

typedef struct {
    long mtype;
    int success;
    char string[MAX_MESSAGE_LENGTH];
} SuccessResponse;

typedef struct {
    long mtype;
    char user[MAX_USERNAME_LENGTH];
    char msg[MAX_MESSAGE_LENGTH];
} Message_to_user;