
INFO: W pliku common.h zdefiniowane są słowa kluczowe i stałe, pod które kompilator podczas kompilacji podstawia odpowiednie wartości. Nazwy te charakteryzują się tym, że pisane są konwencją snake_case, ale przy użyciu wersalików.

W tym samym pliku umieszczone są wszystkie struktury, które używane są do komunikacji między procesami. Struktury do użytku wewnętrznego procesów znajdują się w plikach *.c tych konkretnych procesów.

  

#

  

Wykorzystywane struktury w komunikacji między klientem a serwerem:

  

#

```

typedef struct {

long mtype;

int pid;

char username[MAX_USERNAME_LENGTH];

char password[MAX_PASSWORD_LENGTH];

} User;

```

  

Struktura User to struktura, w której zawiera się informacje dot. użytkownika, który wykonał akcję logowania lub wylogowania. Podczas logowania klient do struktury w pole mtype wpisuje zdefiniowaną wartość pod słowem kluczowym `PROT_LOGIN`, do pola pid przypisuje ID swojego procesu. Pod pole username i password, które są tablicami charów o rozmiarach `MAX_USERNAME_LENGTH i MAX_PASSWORD_LENGTH` kolejno nazwę użytkownika i hasło.

Przy wylogowywaniu natomiast uzupełniane są tylko mtype wartością `PROT_LOGOUT`, pid ponownie ID procesu klienta. Więcej informacji nie jest potrzebnych, ponieważ struktura ta wysyłana jest na prywatną kolejkę identyfikowaną przez PID klienta i to ona jednoznacznie określa, od jakiego użytkownika serwer otrzymał polecenie wylogowania.

#

```

typedef struct{

long mtype;

char string[MAX_MESSAGE_LENGTH];

} Message;

```

  

Struktura Message ma o wiele szersze zastosowanie w tym projekcie. Zawiera ona tylko dwa pola są nimi mtype, w które proces wysyłający komunikat wpisuje typ komunikatu oraz pole string będące tablicą charów o wielkości `MAX_MESSAGE_LENGTH`. Struktura ta wykorzystywana jest przy pięciu zapytaniach wysyłanych od klienta do serwera. Pierwsze dwa: sprawdzenie zalogowanych użytkowników oraz sprawdzenie istniejących grup sprowadza się do wpisania przez klienta do struktury tylko typu komunikatu do pola mtype (kolejno: `PROT_CHECK_LOGGEDIN_REQUEST, PROT_CHECK_GROUPS_REQUEST`). Następuje wysłanie komunikatu do serwera. Serwer do pola string wpisuje odpowiednie dane i odsyła tę samą strukturę do użytkownika z typami kolejno: `PROT_CHECK_LOGGEDIN_RESPONSE, PROT_CHECK_GROUPS_RESPONSE`. Rolą klienta jest tylko wyświetlenie pola string struktury, bo serwer nie umieszcza tam tylko surowej informacji zwrotnej, lecz odpowiednio spreparowaną odpowiedź do użytkownika. Trzy ostatnie akcje tj. sprawdzenie użytkowników w konkretnej grupie, zapisanie użytkownika do grupy oraz wypisanie użytkownika do grupy działają w niemal identycznym schemacie do funkcjonalności opisanych wyżej z tą różnicą, że klient musi do pola string przekazać nazwę grupy, której ma dotyczyć się akcja. Typy komunikatów wysyłanych przez klienta to kolejno `PROT_CHECK_USERS_IN_GROUP_REQUEST, PROT_ENROLL_TO_GROUP_REQUEST, PROT_UNENROLL_FROM_GROUP_REQUEST`. A typy komunikatów odbieranych to kolejno: `PROT_CHECK_USERS_IN_GROUP_RESPONSE, PROT_ENROLL_TO_GROUP_RESPONSE, PROT_UNENROLL_FROM_GROUP_RESPONSE`.

#

```

typedef struct {

long mtype;

int success;

char string[MAX_MESSAGE_LENGTH];

} SuccessResponse;

```

  

Struktura SuccessResponse składa się z pól mtype co oznacza typ komunikatu (long — liczba całkowita), success (liczba całkowita; 0 - gdy akcja wykonywana na serwerze zakończyła się niepowodzeniem lub 1, gdy zakończyła się powodzeniem) oraz pole string będące tablicą charów o długości `MAX_MESSAGE LENGTH`. Struktura ta wykorzystywana jest jedynie podczas procedury logowania. Strukturę tę wysyła tylko klient i jedyny wpisywany typ do pola mtype to PID klienta, do którego ma trafić informacja zwrotna. W polu string mogą pojawić się trzy różne komunikaty. Są to: `"User already logged in"`. `"User blocked (too many unsuccessful login attempts - contact with administrator)"`, `"Unsuccessful attempts for user %s (%d/%d)"`.

#

```

typedef struct {

long mtype;

char user[MAX_USERNAME_LENGTH];

char msg[MAX_MESSAGE_LENGTH];

} Message_to_user;

```

  

Message_to_user jest ostatnią strukturą wykorzystywaną w komunikacji międzyprocesorowej. Jej pola to całkowitoliczbowy mtype zawierający typ komunikatu, user oraz msg, które są tablicami charów o wielkościach kolejno `MAX_USERNAME_LENGTH, MAX_MESSAGE_LENGTH`. Struktura ta wykorzystywana jest podczas dwóch akcji wykonywanych przez klienta. Jest to po pierwsze wysłanie bezpośredniej wiadomości do użytkownika. Klient ustawia mtype na `PROT_SEND_MESSAGE_TO_USER_FROM`, user na nazwę użytkownika, do którego chce wysłać wiadomość oraz do pola msg treść wiadomości. Serwer przechwytuje wysłaną do niego strukturę oraz wykonuje następujące akcje: do pola user wpisuje nazwę użytkownika nadającego wiadomość a pole mtype ustawia na `PROT_SEND_MESSAGE_TO_USER_TO`. Tak przygotowaną strukturę wysyła do użytkownika, do którego kierowana była wiadomość. Jeżeli wiadomość udało się wysłać, to wykorzystując tę samą strukturę do użytkownika, który wiadomość wysyłał, odsyła wiadomość zwrotną. Pod pole msg wpisuje `"Message sent succcesfully"`, a pod mtype `PROT_SEND_MESSAGE_TO_USER_RESPONSE`.

Drugą funkcją, w której wykorzystywana jest owa struktura to wysyłanie wiadomości do grupy. Działa ona jednak na takiej zasadzie, że klient 'wypytuje' serwer, jacy użytkownicy należą do grupy, do której klient chce wysłać wiadomość. Następnie w funkcji for wysyła on wiadomości do poszczególnych użytkowników na zasadzie 'direct message' — czyli wykorzystując funkcjonalność opisaną wyżej.

#

  

Tak prezentują się zdefiniowane w pliku common.h wartości wykorzystywane do określenia typów komunikatów:

```

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

```

  

A tak stałe wykorzystywane do określenia głównie rozmiarów tablic, ale także do określenia ID głównej kolejki, maksymalnej ilości prób, po jakiej użytkownik zostaje zablokowany oraz nazwę pliku konfiguracyjnego:

```

#define MAIN_QUEUE_HEX 0x2137

#define MAX_UNSUCCESSFUL_LOGIN_ATTEMPTS 3 // it must be <=9

#define CONFIG_FILE "config.txt"

```