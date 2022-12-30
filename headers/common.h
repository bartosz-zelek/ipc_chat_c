#define MAIN_QUEUE_ID 0x2137
#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32

#define PROT_LOGIN 1
#define PROT_LOGOUT 2

struct User {
    long mtype;
    int pid;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
};

struct LoginResponse {
    long mtype;
    int success;
};