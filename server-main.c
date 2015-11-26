#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

const int MAX_CONNECTIONS = 20;

void* input(void* param) {
	int* fd = (int*) param;
	char buffer[256];
	do {
		printf("Enter what you want to say > ");
		fgets(buffer, 255, stdin);
		write((*fd), buffer, 255);
	} while (strncmp(buffer, "exit", 4) != 0);
	return NULL;
}
void* output(void* param) {
	int* fd = (int*) param;
	char buffer[256];
	read((*fd), buffer, 255);
	while (strncmp(buffer, "exit", 4) != 0) {
		printf("\nClient > %s\nEnter what you want to say > ", buffer);
		read((*fd), buffer, 255);
	}
	return NULL;
}
int main() {
	struct addrinfo request;
	request.ai_flags = AI_PASSIVE;
	request.ai_family = AF_INET;
	request.ai_socktype = SOCK_STREAM;
	request.ai_protocol = 0;
	request.ai_addrlen = 0;
	request.ai_addr = NULL;
	request.ai_canonname = NULL;
	request.ai_next = NULL;
	struct addrinfo* result;
	getaddrinfo(NULL, "63777", &request, &result);
	int sd;
	char buffer[256];
	if ((sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) >= 0) {
		int on = 1;
		if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == 0) {
			if (bind(sd, result->ai_addr, result->ai_addrlen) == 0) {
				listen(sd, MAX_CONNECTIONS);
				struct sockaddr_in senderAddr;
				pthread_t inputThread[MAX_CONNECTIONS], outputThread[MAX_CONNECTIONS];
				int ic = sizeof(senderAddr);
				int connectionNum = 0;
				int fd;
				printf("Waiting for client to connect...\n");
				while ((fd = accept(sd, (struct sockaddr*) &senderAddr, &ic)) != 0) {
					printf("\nClient connected!\n");
					if (pthread_create(&inputThread[connectionNum], NULL, input, &fd) != 0) {
						printf("Error creating thread. Exiting.\n");
						return -1;
					}
					if (pthread_create(&outputThread[connectionNum], NULL, output, &fd) != 0) {
						printf("Error creating thread. Exiting.\n");
						return -1;
					}
				}
				//pthread_join() here?
			} else {
				printf("Socket could not be bound to address. Exiting.\n");
				freeaddrinfo(result);
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