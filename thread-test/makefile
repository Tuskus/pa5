cc = gcc
all : thread-test clean
thread-test : thread-test.c
	$(cc) -pthread -o thread-test thread-test.c
clean:
	rm -rf *.o