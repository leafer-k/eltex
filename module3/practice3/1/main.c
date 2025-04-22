#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_NUMS 1000
#define SAVE_FILE "save.txt"

void read_file() {
	FILE* file = fopen(SAVE_FILE, "r");
	if (!file) {
		perror("fopen read");
		return;
	}

	printf("File contents:\n");
	char buffer[256];
	while (fgets(buffer, sizeof(buffer), file)) {
		printf("%s", buffer);
	}
	fclose(file);
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Invalid arguments. Required 1, provided %d\n", argc-1);
		exit(EXIT_FAILURE);
	}

	sem_t* semid;

	if((semid = sem_open("/sem", O_CREAT, 0666, 1)) == SEM_FAILED) {
		perror("semget");
		exit(EXIT_FAILURE);
	}

	FILE* clear_file = fopen(SAVE_FILE, "w");
	if (!clear_file) {
		perror("Failed to clear file");
		exit(EXIT_FAILURE);
	}
	fclose(clear_file);

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
			int temp;
			sem_getvalue(semid, &temp);
			if(sem_wait(semid) == -1) {
				perror("sem_wait");
				break;
			}

			read_file();

			if(sem_post(semid) == -1) {
				perror("sem_post");
				break;
			}


			int num = rand() % 1000;
			if (write(fd[1], &num, sizeof(num)) != sizeof(num)) {
				perror("write");
				break;
			}
			usleep(5e5);
		}
		close(fd[1]);
		exit(EXIT_SUCCESS);

	} else {
		close(fd[1]);
		FILE* fd_out;

		printf("Generated numbers:\n");
		int num;
		for(int i = 0; i < n; i++) {
			if(read(fd[0], &num, sizeof(num)) != sizeof(num)) {
				perror("read");
				fclose(fd_out);
				break;
			}

			if(sem_wait(semid) == -1) {
				perror("semop lock");
				break;
			}

			fd_out = fopen(SAVE_FILE, "a");

			if(!fd_out) {
				perror("open");
				break;
			}

			printf("%d ", num);
			fprintf(fd_out, "%d\n", num);
			fflush(fd_out);

			fclose(fd_out);

			if(sem_post(semid) == -1) {
				perror("semop unlock");
				break;
			}
			usleep(5e5);
		}
		printf("\n");
		close(fd[0]);
		wait(NULL);

        sem_close(semid);
		sem_unlink("/sem");
	}
	exit(EXIT_SUCCESS);
}
