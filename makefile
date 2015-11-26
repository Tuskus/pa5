cc = gcc
all : server-main client-main clean
server-main : server-main.c
	$(cc) -pthread -o server-main server-main.c
client-main : client-main.c
	$(cc) -pthread -o client-main client-main.c
clean:
	rm -rf *.o