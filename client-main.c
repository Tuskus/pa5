#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

struct addrinfo* result;

void clearBuffer(char* b) {
	int i = 0;
	while (i < 256) {
		b[i] = '\0';
		i++;
	}
}
void* input(void* param) {
	int* sd = (int*) param;
	char buffer[256];
	clearBuffer(buffer);
	write((*sd), "hello", 256);
	do {
		clearBuffer(buffer);
		printf("\n> ");
		fgets(buffer, 256, stdin);
		write((*sd), buffer, 256);
	} while (strncmp(buffer, "exit", 4) != 0);
	exit(0);
	freeaddrinfo(result);
	return NULL;
}
void* output(void* param) {
	int* sd = (int*) param;
	char buffer[256];
	read((*sd), buffer, 255);
	while (strncmp(buffer, "exit", 4) != 0) {
		printf("%s\n> ", buffer);
		read((*sd), buffer, 255);
	}
	return NULL;
}
int main(int argc, char** argv) {
	if (argc < 2) {
		printf("You need to enter the address of the server. Exiting.\n");
		return -1;
	}
	struct addrinfo request;
	request.ai_flags = 0;
	request.ai_family = AF_INET;
	request.ai_socktype = SOCK_STREAM;
	request.ai_protocol = 0;
	request.ai_addrlen = 0;
	request.ai_addr = NULL;
	request.ai_canonname = NULL;
	request.ai_next = NULL;
	getaddrinfo(argv[1], "63777", &request, &result);
	int sd;
	if ((sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) >= 0) {
		int on = 1;
		if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == 0) {
			do {
				connect(sd, result->ai_addr, result->ai_addrlen);
				printf("Attempting to connect to bank...\n");
			} while(errno == ECONNREFUSED);
			printf("Connected to bank server!\n");
			pthread_t inputThread, outputThread;
			if (pthread_create(&inputThread, NULL, input, &sd) != 0) {
				printf("Error creating thread. Exiting.\n");
				return -1;
			}
			if (pthread_create(&outputThread, NULL, output, &sd) != 0) {
				printf("Error creating thread. Exiting.\n");
				return -1;
			}
			if (pthread_join(inputThread, NULL) != 0) {
				printf("Error joining thread. Exiting.\n");
				return -1;
			}
			if (pthread_join(outputThread, NULL) != 0) {
				printf("Error joining thread. Exiting.\n");
				return -1;
			}
		} else {
			printf("Socket options could not be set. Exiting.\n");
			freeaddrinfo(result);
			return -1;
		}
	} else {
		printf("Socket could not be opened. Exiting.\n");
		freeaddrinfo(result);
		return -1;
	}
	freeaddrinfo(result);
	return 0;
}