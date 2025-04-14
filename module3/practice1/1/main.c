#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


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
			side = atof(argv[i]);
			sqr = side*side;
			printf("(%d) Child: %g\n", getpid(), sqr);
		}
	} else {
		for(int i = argc/2 + 1; i < argc; i++) {
			side = atof(argv[i]);
			sqr = side*side;
			printf("(%d) Parent: %g\n", getpid(), sqr);
		}
	}

	exit(EXIT_SUCCESS);
}
