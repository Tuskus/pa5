#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

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
	struct addrinfo* result;
	getaddrinfo(argv[1], "63777", &request, &result);
	int sd;
	char buffer[256] = "Hello from the client!";
	if ((sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) >= 0) {
		int on = 1;
		if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == 0) {
			do {
				connect(sd, result->ai_addr, result->ai_addrlen);
				printf("Still not connected...\n");
			} while(errno == ECONNREFUSED);
			printf("Connected to server!\n");
			do {
				printf("Enter what you want to say > ");
				fgets(buffer, 255, stdin);
				write(sd, buffer, 255);
			} while (strncmp(buffer, "exit", 4) != 0);
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