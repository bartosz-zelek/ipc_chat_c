## ipc_chat_c

To compile the program, execute the following commands in the project directory:

```bash
gcc -o server.out inf151860_151765_s.c inf151860_151765_c.c
gcc -o client.out inf151860_151765_k.c
```

To run the client, use the command: `./client.out`

To run the server, use the command: `./server.out`

Alternatively, you can run the script `./comp.sh`, which automatically compiles the project files and starts the server process.

## File Description

- `inf151860_151765_k.c`: This file contains the client functionality. It enables communication with the server by opening a public queue to the server. After logging in, it communicates only through a private queue created by the server. It includes functions with self-explanatory names such as `login_procedure`, `logout_procedure`, `check_loggedin_users`, `check_groups`, `check_users_in_group`, `enroll_to_group`, `unenroll_from_group`, `send_message_to_user`, `receive_message_from_user`, `send_message_to_group`. In brief, the client's role is to pass user commands to the server and display the results of these commands.
- `inf151860_151765_s.c`: This file contains the server functionality. It implements the main functionality of the project. The program starts by retrieving information from a configuration file, which is the only persistent data. The server treats it as a non-modifiable file, and all data generated during the program's execution are stored in RAM, existing as long as the server is running. The heart of the program is an infinite `while` loop where functions like `catch_and_perform_[...]_action` are called. These functions capture commands from users and execute appropriate actions.
