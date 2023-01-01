#pragma once

#define MAIN_QUEUE_HEX 0x2137
#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32
#define MAX_MESSAGE_LENGTH 1000
#define MAX_GROUP_NAME_LENGTH 32
#define MAX_GROUPS 10
#define MAX_LINE_LENGTH 500

#define PROT_LOGIN 1
#define PROT_LOGOUT 2
#define PROT_CHECK_LOGGEDIN_REQUEST 3
#define PROT_CHECK_LOGGEDIN_RESPONSE 4
#define PROT_CHECK_GROUPS_REQUEST 5
#define PROT_CHECK_GROUPS_RESPONSE 6

#define CONFIG_FILE "config.txt"

typedef struct {
    long mtype;
    int pid;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} User;

typedef struct{
    long mtype;
    char string[MAX_MESSAGE_LENGTH];
} Message;

typedef struct {
    long mtype;
    int success;
} LoginResponse;