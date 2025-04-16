#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_NUMS 1000
#define SAVE_FILE "save.txt"

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Invalid arguments. Required 1, provided %d\n", argc-1);
		exit(EXIT_FAILURE);
	}

	int n = atoi(argv[1]);

	if(n > MAX_NUMS || n == 0) {
		printf("Invalid argument\n");
		exit(EXIT_FAILURE);
	}

	int fd[2];

	if(pipe(fd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();

	if(pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if(pid == 0) {
		close(fd[0]);
		srand(time(NULL));

		for (int i = 0; i < n; i++) {
			int num = rand() % 1000;
			if (write(fd[1], &num, sizeof(num)) != sizeof(num)) {
				perror("write");
				close(fd[1]);
				exit(EXIT_FAILURE);
			}
		}
		close(fd[1]);
		exit(EXIT_SUCCESS);

	} else {
		close(fd[1]);
		FILE *fd_out = fopen(SAVE_FILE, "w");

		if(!fd_out) {
			perror("open");
			close(fd[0]);
			exit(EXIT_FAILURE);
		}

		printf("Generated numbers:\n");
		int num;
		for(int i = 0; i < n; i++) {
			if(read(fd[0], &num, sizeof(num)) != sizeof(num)) {
				perror("read");
				close(fd[0]);
				fclose(fd_out);
				exit(EXIT_FAILURE);
			}

			printf("%d ", num);
			fprintf(fd_out, "%d\n", num);
		}
		printf("\n");

		close(fd[0]);
		fclose(fd_out);

		wait(NULL);
	}

	exit(EXIT_SUCCESS);
}
