#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>

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

	key_t key = ftok(".", 1);
	int semid;

	if((semid = semget(key, 1, IPC_CREAT | 0666)) == -1) {
		perror("semget");
		exit(EXIT_FAILURE);
	}

	if(semctl(semid, 0, SETVAL, 1) == -1) {
		perror("semctl");
		exit(EXIT_FAILURE);
	}

	struct sembuf lock = {0, -1, 0};
	struct sembuf unlock[2] = {{0, 0, 0}, {0, 1, 0}};

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

			if(semop(semid, &lock, 1) == -1) {
				perror("semop lock");
				break;
			}

			read_file();

			if(semop(semid, unlock, 2) == -1) {
				perror("semop unlock");
				break;
			}


			int num = rand() % 1000;
			if (write(fd[1], &num, sizeof(num)) != sizeof(num)) {
				perror("write");
				break;
			}
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

			if(semop(semid, &lock, 1) == -1) {
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
			fclose(fd_out);
			if(semop(semid, unlock, 2) == -1) {
				perror("semop unlock");
				break;
			}
		}
		printf("\n");
		close(fd[0]);
		wait(NULL);

        if (semctl(semid, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID");
        }
	}
	exit(EXIT_SUCCESS);
}
