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

pthread_mutex_t createLock;
char buffer[105];
SortedListPtr accountList;

void clearBuffer(char* b) {
	int i = 0;
	while (i < 105) {
		b[i] = '\0';
		i++;
	}
}
int accountCmp(void* arg1, void* arg2) {
	account_t* a1 = (account_t*) arg1;
	account_t* a2 = (account_t*) arg2;
	return strcmp(a1->name, a2->name);
}
void accountDestroy(void* arg) {
	account_t* a = (account_t*) arg;
	ADestroy(a);
}
void open(char* accountname, int fd) {
	int endIndex = 0;
	while (isalpha(accountname[endIndex]) != 0) {
		endIndex++;
	}
	if (endIndex == 0) {
		sprintf(buffer, "Error: Invalid account name. New account not created.\n");
		write(fd, buffer, 105);
		return;
	}
	accountname[endIndex] = '\0';
	char nameFormatted[endIndex + 1];
	strncpy(nameFormatted, accountname, endIndex + 1);
	pthread_mutex_lock(&createLock);
	SortedListIteratorPtr iter = SLCreateIterator(accountList);
	account_t* currentAccount = SLGetItem(iter);
	while (currentAccount != NULL) {
		if (strcmp(currentAccount->name, nameFormatted) == 0) {
			sprintf(buffer, "Error: Account name \"%s\" already exists. New account not created.\n", nameFormatted);
			write(fd, buffer, 105);
			SLDestroyIterator(iter);
			pthread_mutex_unlock(&createLock);
			return;
		} else if (strcmp(currentAccount->name, "") == 0) {
			ASetName(currentAccount, nameFormatted);
			currentAccount->balance = 0.00;
			sprintf(buffer, "Account name \"%s\" successfully created.\n", nameFormatted);
			write(fd, buffer, 105);
			SLDestroyIterator(iter);
			pthread_mutex_unlock(&createLock);
			return;
		}
		currentAccount = SLNextItem(iter);
	}
	SLDestroyIterator(iter);
	pthread_mutex_unlock(&createLock);
	sprintf(buffer, "Error: Too many accounts. New account not created.\n", nameFormatted);
	write(fd, buffer, 105);
}
account_t* start(char* accountname, int fd) {
	int endIndex = 0;
	while (isalpha(accountname[endIndex]) != 0) {
		endIndex++;
	}
	if (endIndex == 0) {
		sprintf(buffer, "Error: Invalid account name. Session not started.\n");
		write(fd, buffer, 105);
		return NULL;
	}
	accountname[endIndex] = '\0';
	char nameFormatted[endIndex + 1];
	strncpy(nameFormatted, accountname, endIndex + 1);
	SortedListIteratorPtr iter = SLCreateIterator(accountList);
	account_t* currentAccount = SLGetItem(iter);
	while (currentAccount != NULL) {
		if (strcmp(currentAccount->name, nameFormatted) == 0) {
			if (currentAccount->inSession == 0) {
				write(fd, "Error: Account is already in session. Session not started.\n", 105);
				return NULL;
			} else {
				sprintf(buffer, "Session started for account \"%s\".\n", nameFormatted);
				write(fd, buffer, 105);
				SLDestroyIterator(iter);
				pthread_mutex_unlock(&createLock);
				return currentAccount;
			}
		}
		currentAccount = SLNextItem(iter);
	}
	SLDestroyIterator(iter);
	sprintf(buffer, "Error: Account \"%s\" not found. Session not started.\n", nameFormatted);
	write(fd, buffer, 105);
	return NULL;
}
void* session(void* param) {
	account_t* currentAccount = NULL;
	int* fd = (int*) param;
	clearBuffer(buffer);
	read((*fd), buffer, 105);
	while (strncmp(buffer, "exit", 4) != 0) {
		clearBuffer(buffer);
		read((*fd), buffer, 105);
		if (strncmp(buffer, "open ", 5) == 0) {
				open(buffer + 5, (*fd));
		} else if (strncmp(buffer, "start ", 6) == 0) {
			if (currentAccount == NULL) {
				currentAccount = start(buffer + 6, (*fd));
			} else {
				write((*fd), "Error: User is already in session with another account. Session not started.\n", 105);
			}
		} else if (strncmp(buffer, "credit ", 7) == 0) {
			// tell client command could not be completed if accountIndex is not in correct range
			// in a seperate function:
				// check to make sure number is floating point
				// add number to balance
		} else if (strncmp(buffer, "debit ", 6) == 0) {
			// tell client command could not be completed if accountIndex is not in correct range
			// complain if amount to subtract is greater than current balance
			// use same function as credit if it checks out
		} else if (strncmp(buffer, "balance", 7) == 0) {
			if (currentAccount == NULL) {
				write((*fd), "Error: User is not currently in a session.\n", 105);
			} else {
				sprintf(buffer, "Account \"%s\" has $%f\n", currentAccount->name, currentAccount->balance);
				write((*fd), buffer, 105);
			}
		} else if (strncmp(buffer, "finish", 6) == 0) {
			if (currentAccount == NULL) {
				write((*fd), "Error: User is not currently in a session.\n", 105);
			} else {
				sprintf(buffer, "Account \"%s\" closed.\n", currentAccount->name);
				write((*fd), buffer, 105);
				currentAccount = NULL;
			}
		}
	}
	if (currentAccount != NULL) {
		currentAccount->inSession = 1;
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
	if (pthread_mutex_init(&createLock, 0) != 0) {
		printf("Error creating mutex lock. Exiting.\n");
		return -1;
	}
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