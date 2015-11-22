cc = gcc
all : server-main client-main clean
server-main : server-main.c
	$(cc) -o server-main server-main.c
client-main : client-main.c
	$(cc) -o client-main client-main.c
clean:
	rm -rf *.o