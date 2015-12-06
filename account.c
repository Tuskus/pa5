#include "account.h"
#include <stdlib.h>
#include <string.h>

account_t* ACreate(char* newName, float money) {
	account_t* result = (account_t*) malloc(sizeof(account_t*));
	result->name = (char*) malloc(strlen(newName + 1));
	strcat(result->name, newName);
	strcat(result->name, "\0");
	result->balance = money;
	result->inSession = 1;
	return result;
}

void ASetName(account_t* a, char* newName) {
	free(a->name);
	a->name = (char*) malloc(strlen(newName + 1));
	strcat(a->name, newName);
}

void ADestroy(account_t* a) {
	free(a->name);
	free(a);
}