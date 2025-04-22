#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>

#define MAX_NUMS 1000
#define MAX_READING_PROCESSES 3
#define SAVE_FILE "save.txt"

void read_file() {
	FILE* file = fopen(SAVE_FILE, "r");
	if (!file) {
		perror("fopen read");
		return;
	}

	printf("[%d] File contents:\n", getpid());
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), file)) {
		printf("%s\n", buffer);
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

	if((semid = semget(key, 2, IPC_CREAT | 0666)) == -1) {
		perror("semget");
		exit(EXIT_FAILURE);
	}

	if(semctl(semid, 0, SETVAL, 0) == -1) {
		perror("semctl");
		exit(EXIT_FAILURE);
	}

	if(semctl(semid, 1, SETVAL, MAX_READING_PROCESSES) == -1) {
		perror("semctl");
		exit(EXIT_FAILURE);
	}

	struct sembuf write_lock[3] = {{0, 0, 0}, {0, 1, 0}, {1, MAX_READING_PROCESSES, 0}};
	struct sembuf write_unlock = {0, -1, 0};

	struct sembuf read_lock[2] = {{1, -1, 0}, {0, 0, 0}};
	struct sembuf read_unlock = {1, 1, 0};

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

	//Для теста, что работает с несколькими процессами
/*
	for(int i = 0; i < 3; i++) {
		pid = fork();

		if(pid == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}
	}
*/


	if(pid == 0) {
		close(fd[0]);
		srand(time(NULL));

		for (int i = 0; i < n; i++) {

			printf("Read lock...\n");
			if(semop(semid, read_lock, 2) == -1) {
				perror("semop read_lock");
				break;
			}
			printf("Read locked!\n\n");

			read_file();
			printf("Read unlock...\n");
			if(semop(semid, &read_unlock, 1) == -1) {
				perror("semop read_unlock");
				break;
			}
			printf("Read unlocked!\n\n");

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

			printf("Write lock...\n");
			if(semop(semid, write_lock, 3) == -1) {
				perror("semop write_lock");
				break;
			}
			printf("Write locked!\n\n");
			fd_out = fopen(SAVE_FILE, "a");

			if(!fd_out) {
				perror("open");
				break;
			}

			printf("Writing: %d\n", num);
			fprintf(fd_out, "%d ", num);
			fclose(fd_out);

			printf("Write unlock...\n");
			if(semop(semid, &write_unlock, 1) == -1) {
				perror("semop write_unlock");
				break;
			}
			printf("Write unlocked!\n\n");
			usleep(5e5);
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
