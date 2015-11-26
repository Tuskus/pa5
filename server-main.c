#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#define MAX_CONNECTIONS 20

struct account {
	char* name;
	float balance;
	int inSession : 1;
};
typedef struct account account_t;

void ACreate(account_t* account, char* str, float money) {
	account->name = str;
	account->balance = money;
	account->inSession = 0;
	printf("account.name = \"%s\"\n", account->name);
}

char buffer[256];
account_t accounts[MAX_CONNECTIONS];

void open(char* accountname, int fd) {
	int endIndex = 0;
	while (isalpha(accountname[endIndex]) != 0 && endIndex < 100) {
		endIndex++;
	}
	char nameFormatted[endIndex + 1];
	strncat(nameFormatted, accountname, endIndex);
	nameFormatted[endIndex] = 0x00;
	int i = 0, accountNum = -1;
	while (i < MAX_CONNECTIONS) {
		if (strcmp(nameFormatted, accounts[i].name) == 0) {
			sprintf(buffer, "Error: Account name \"%s\" alreayd exists. New account not created.\n", nameFormatted);
			write(fd, buffer, 255);
			return;
		} else if (accountNum == -1 && strcmp(accounts[i].name, "") == 0) {
			accountNum = i;
		}
		i++;
	}
	if (accountNum > 0 && accountNum < MAX_CONNECTIONS) {
		ACreate(&(accounts[accountNum]), nameFormatted, 0.00);
	}
	// account creation not working. malloc?
	// lock for client creation
	// don't allow this if client is already in a session
	printf("opening account: \"%s\"\n", nameFormatted);
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
	read((*fd), buffer, 255);
	while (strncmp(buffer, "exit", 4) != 0) {
		read((*fd), buffer, 255);
		if (strncmp(buffer, "open ", 5) == 0) {
			open(buffer + 5, (*fd));
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
	int i = 0;
	while (i < MAX_CONNECTIONS) {
		ACreate(&accounts[i], "", 0.00);
		i++;
	}
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