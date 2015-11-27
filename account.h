#ifndef ACCOUNT_H
#define ACCOUNT_H

struct account {
	char* name;
	float balance;
	int inSession : 1;
};
typedef struct account account_t;
account_t* ACreate(char* newName, float money);
void ASetName(account_t* a, char* newName);
void ADestroy(account_t* a);

#endif