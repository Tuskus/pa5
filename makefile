cc = gcc
all : libaccount.a account.o libsl.a sorted-list.o server client clean
server : server.c
	$(cc) -pthread -o server server.c libaccount.a libsl.a
client : client.c
	$(cc) -pthread -o client client.c
libaccount.a : account.o
	ar rv libaccount.a account.o
account.o : account.c account.h
	$(cc) -c account.c
libsl.a : sorted-list.o
	ar rv libsl.a sorted-list.o
sorted-list.o : sorted-list.c sorted-list.h
	$(cc) -c sorted-list.c
clean:
	rm -rf *.o