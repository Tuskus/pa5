#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

const int MAX_CONNECTIONS = 20;

struct account {
	char* name;
	float balance;
	int inSession : 1;
};
typedef struct account account_t;

void open(char* accountname) {
	// get real account name (no whitespace), cap at 100 chars
	// throw error if account list is full
	// throw error if accountname already exists
	// don't allow this if client is already in a session
	// set name, balance to zero
	// tell client command could not be completed if accountIndex is not in correct range
	// lock for client creation
	printf("opening account: \"%s\"\n", accountname);
}
void start(char* accountname) {
	// set accountIndex for thread (pass that var as a pointer?)
	// tell client command could not be completed if accountIndex is not in correct range
	// check for concurrent sessions for same account (use inSession probably )
	printf("starting session for account: \"%s\"", accountname);
}
void* session(void* param) {
	int accountIndex = -1;
	int* fd = (int*) param;
	char buffer[256];
	read((*fd), buffer, 255);
	while (strncmp(buffer, "exit", 4) != 0) {
		read((*fd), buffer, 255);
		if (strncmp(buffer, "open ", 5) == 0) {
			open(buffer + 5);
		} else if (strncmp(buffer, "start ", 6) == 0) {
			start(buffer + 6);
		} else if (strncmp(buffer, "credit ", 7) == 0) {
			// tell client command could not be completed if accountIndex is not in correct range
			// in a seperate function:
				// check to make sure number is floating point
				// add number to balance
		} else if (strncmp(buffer, "debit ", 6) == 0) {
			// tell client command could not be completed if accountIndex is not in correct range
			// complain if amount to subtract is greater than current balance
			// use same function as credit if it checks out
		} else if (strncmp(buffer, "balance ", 8) == 0) {
			// tell client command could not be completed if accountIndex is not in correct range
			// send client balance number
		} else if (strncmp(buffer, "finish ", 7) == 0) {
			// tell client command could not be completed if accountIndex is not in correct range
			// set accountIndex to -1
		}
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
	account_t accounts[20];
	if ((sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) >= 0) {
		int on = 1;
		if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == 0) {
			if (bind(sd, result->ai_addr, result->ai_addrlen) == 0) {
				listen(sd, MAX_CONNECTIONS);
				struct sockaddr_in senderAddr;
				pthread_t sessionThread[MAX_CONNECTIONS];
				int ic = sizeof(senderAddr);
				int connectionNum = 0;
				int fd;
				printf("Waiting for client to connect...\n");
				while ((fd = accept(sd, (struct sockaddr*) &senderAddr, &ic)) != 0) {
					printf("\nClient connected!\n");
					if (pthread_create(&sessionThread[connectionNum], NULL, session, &fd) != 0) {
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