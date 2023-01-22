
# ipc_chat_c

  

Aby skompliować program należy wykonać następujące komendy w katalogu projektu:

`gcc -o server.out server.c config.c`
`gcc -o client.out client.c`

Uruchomienie klienta to komenda: `./client.out`

Uruchomienie serwera to komenda: `./server.out`

  

Alternatywnie można uruchomić skrypt `./comp.sh`, który automatycznie skompiluje pliki projektu oraz uruchomi proces serwera.
#

Opis plików:

`client.c` - plik z funkcjonalnością klienta. Umożliwia komunikację z serwerem poprzez otworzenie publicznej kolejki do serwera. Po zalogowaniu się z serwerem komunikuje się już tylko poprzez prywatną kolejkę stworzoną przez serwer. Zawiera następujące funkcje o następujących nazwach które same w sobie wyjaśniają ich funkcjonalność: `login_procedure, logout_procedure, check_loggedin_users, check_groups, check_users_in_group, enroll_to_group, unenroll_from_group, send_message_to_user, revieve_message_from user, send_message_to_group`. W dużym skrócie rola klienta to przekazywanie poleceń użytkownika do serwera i wyświetlanie wyników tychże poleceń.

  

`server.c` - plik z funkcjonalnościa serwera. Tu zaimplementowana jest główna funkcjonalność projektu. Program rozpoczyna się od pobrania informacji z pliku konfiguracyjnego. Są to jedyne dane trwałe. Serwer naturalnie traktuje go jako plik niemodyfikowalny. A wszystkie dane powstałe podczas działania programu zapisywana są w pamięci RAM ergo istnieją tak długo jak długo działa uruchomiony serwer. Sercem programu jest nieskończona pętla while. Tam wywoływane są funkcje typu `catch_and_perform_[...]_action`. Funkcje te przechwytują polecenia od uzytkowników i uruchamiają odpowiednie akcje.
#