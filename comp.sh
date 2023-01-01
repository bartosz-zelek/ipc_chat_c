# the script to compile the server and client
# automatically run the server
gcc -o server.out server.c config.c
gcc -o client.out client.c
./server.out