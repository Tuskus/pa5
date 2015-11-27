#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "account.h"
#include "sorted-list.h"
#define MAX_CONNECTIONS 20

char buffer[256];
SortedListPtr accountList;

int accountCmp(void* arg1, void* arg2) {
	account_t* a1 = (account_t*) arg1;
	account_t* a2 = (account_t*) arg2;
	return strcmp(a1->name, a2->name);
}
void accountDestroy(void* arg) {
	account_t* a = (account_t*) arg;
	ADestroy(a);
}
account_t* open(char* accountname, int fd) {
	int endIndex = 0;
	while (isalpha(accountname[endIndex]) != 0 && endIndex < 100) {
		endIndex++;
	}
	char nameFormatted[endIndex + 1];
	strncat(nameFormatted, accountname, endIndex);
	nameFormatted[endIndex] = 0x00;
	SortedListIteratorPtr iter = SLCreateIterator(accountList);
	account_t* currentAccount = SLGetItem(iter);
	while (currentAccount != NULL) {
		if (strcmp(currentAccount->name, accountname) == 0) {
			sprintf(buffer, "Error: Account name \"%s\" already exists. New account not created.\n", nameFormatted);
			write(fd, buffer, 255);
			SLDestroyIterator(iter);
			return NULL;
		} else if (strcmp(currentAccount->name, "") == 0) {
			ASetName(currentAccount, accountname);
			currentAccount->balance = 0.00;
			// write to client "account created!"
			printf("opening account: \"%s\"\n", nameFormatted);
			SLDestroyIterator(iter);
			return currentAccount;
		}
		SLNextItem(iter);
	}
	SLDestroyIterator(iter);
	// write "accounts full, not created"
	return NULL;
	
	// lock for client creation
	// don't allow this if client is already in a session
	// write to client telling them account was successfully opened
}
void start(char* accountname) {
	// set accountIndex for thread (pass that var as a pointer?)
	// tell client command could not be completed if accountIndex is not in correct range
	// check for concurrent sessions for same account (use inSession probably )
	printf("starting session for account: \"%s\"", accountname);
}
void* session(void* param) {
	account_t* currentAccount = NULL;
	int* fd = (int*) param;
	read((*fd), buffer, 255);
	while (strncmp(buffer, "exit", 4) != 0) {
		read((*fd), buffer, 255);
		if (strncmp(buffer, "open ", 5) == 0) {
			currentAccount = open(buffer + 5, (*fd));
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
	accountList = SLCreate(accountCmp, accountDestroy); 
	int i = 0;
	while (i < MAX_CONNECTIONS) {
		SLInsert(accountList, ACreate("", 0.00));
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