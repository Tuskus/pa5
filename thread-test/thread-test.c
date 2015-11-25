#include <pthread.h>
#include <stdio.h>

void* decrement(void* param) {
	int* num = (int*) param;
	int i = 0;
	while (i < 100) {
		(*num)--;
		printf("Decrementing num: %d\n", (*num));
		i++;
	}
	return NULL;
}
void* increment(void* param) {
	int* num = (int*) param;
	int i = 0;
	while (i < 100) {
		(*num)++;
		printf("Incrementing num: %d\n", (*num));
		i++;
	}
	return NULL;
}
int main() {
	int num = 0;
	pthread_t thread0, thread1;
	if (pthread_create(&thread0, NULL, decrement, &num) != 0) {
		printf("Error creating thread. Exiting.\n");
		return -1;
	}
	if (pthread_create(&thread1, NULL, increment, &num) != 0) {
		printf("Error creating thread. Exiting.\n");
		return -1;
	}
	if (pthread_join(thread0, NULL) != 0) {
		printf("Error joining thread. Exiting.\n");
		return -1;
	}
	if (pthread_join(thread1, NULL) != 0) {
		printf("Error joining thread. Exiting.\n");
		return -1;
	}
	return 0;
}