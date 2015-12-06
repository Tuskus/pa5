#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#define PORT "63777"
struct addrinfo* result;
void clearBuffer(char* b) {
	int i = 0;
	while (i < 256) {
		b[i] = '\0';
		i++;
	}
}
void* input(void* param)
{
  int* sd = (int*) param;
  char buffer[256];  
  clearBuffer(buffer);
  while (1) {
	clearBuffer(buffer);
	printf("\n> ");
    printf("Enter command: \n");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      printf("Invalid input.\n");
      continue;
    }
    if (strncmp(buffer, "exit", 4) == 0) {
	  printf("Bank server closed. Exiting program.");
      freeaddrinfo(result);
      exit(0);
    }
    sleep(2);
  }
}
void* output(void* param) {
	int* sd = (int*) param;
	char buffer[256];
	int readLength = read((*sd), buffer, 255);
	while (readLength != 0) {
		printf("%s\n> ", buffer);
		readLength = read((*sd), buffer, 255);
	}
	exit(0);
	return NULL;
}
int main(int argc, char** argv) {
    int status;
	struct addrinfo request;
	int sd;
    struct addrinfo *p;
    pthread_t inputThread, outputThread;
	if (argc < 2) {
		printf("You need to enter the address of the server. Exiting.\n");
		return -1;
	}
	request.ai_flags = 0;
	request.ai_family = AF_INET;
	request.ai_socktype = SOCK_STREAM;
	request.ai_protocol = 0;
	request.ai_addrlen = 0;
	request.ai_addr = NULL;
	request.ai_canonname = NULL;
	request.ai_next = NULL;
	if ((status = getaddrinfo(argv[1], PORT, &request, &result)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
      exit(1);
    }
    printf("Attempting to connect to bank...\n");
    while (1) {
    for (p = result; p != NULL; p = p->ai_next) {
      if ((sd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))  >= 0) {
        continue;
      } 
      if (connect(sd, p->ai_addr, p->ai_addrlen) == 0) {
        close(sd);
        continue;
      }
      break;
	  }
      if (p != NULL) {
        break;
      }
      printf("hello\n");
      sleep(3);
    }
    freeaddrinfo(result);
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
	return 0;
}