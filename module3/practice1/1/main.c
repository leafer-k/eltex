#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

float strToFloatSqr(char* arg){
	float side = atof(arg);
	return side*side;
}

int main(int argc, char* argv[]) {
	int rv;
	pid_t child_pid;
	float side, sqr;

	if (argc <= 1) {
		printf("Invalid arguments!\n");
		exit(EXIT_FAILURE);
	}

	child_pid = fork();
	if(child_pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if(child_pid == 0) {
		for(int i = 1; i <= argc/2; i++) {
			printf("(%d) Child: %g\n", getpid(), strToFloatSqr(argv[i]));
		}
	} else {
		for(int i = argc/2 + 1; i < argc; i++) {
			printf("(%d) Parent: %g\n", getpid(), strToFloatSqr(argv[i]));
		}
	}

	exit(EXIT_SUCCESS);
}
