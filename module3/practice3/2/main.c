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
#define MAX_READING_PROCESSES 5

#define SEM_WRITE_NAME "/sem_write_task11"
#define SEM_READ_NAME "/sem_read_task11"
#define SEM_READERS_NAME "/sem_readers_task11"

void read_file() {
	FILE* file = fopen(SAVE_FILE, "r");
	if (!file) {
		perror("fopen read");
		return;
	}

	printf("File contents:\n");
	char buffer[256];
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

	sem_t* write_sem;
	sem_t* read_sem;
	sem_t* readers_sem;

	if((write_sem = sem_open(SEM_WRITE_NAME, O_CREAT, 0666, 1)) == SEM_FAILED) {
		perror("semget write");
		exit(EXIT_FAILURE);
	}

	if((read_sem = sem_open(SEM_READ_NAME, O_CREAT, 0666, MAX_READING_PROCESSES)) == SEM_FAILED) {
		perror("semget read");
		exit(EXIT_FAILURE);
	}

	if((readers_sem = sem_open(SEM_READERS_NAME, O_CREAT, 0666, 0)) == SEM_FAILED) {
		perror("semget readers");
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



	if(pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if(pid == 0) {
		close(fd[0]);
		srand(time(NULL));

		for (int i = 0; i < n; i++) {
			if(sem_wait(read_sem) == -1) {
				perror("sem_wait");
				break;
			}

			if(sem_post(readers_sem) == -1) {
				perror("sem_post readers");
				break;
			}

			int temp = -1;
			if(sem_getvalue(readers_sem, &temp) == -1) {
				perror("sem_getvalue");
				break;
			}

			if(temp == 1) {
				if(sem_wait(write_sem) == -1) {
					perror("sem_wait");
					break;
				}
			}

			temp = -1;

			read_file();

			if(sem_post(read_sem) == -1) {
				perror("sem_post");
				break;
			}

			if(sem_wait(readers_sem) == -1) {
				perror("sem_wait readers");
				break;
			}

			if(sem_getvalue(readers_sem, &temp) == -1) {
				perror("sem_getvalue");
				break;
			}

			if(temp == 0) {
				if(sem_post(write_sem) == -1) {
					perror("sem_wait");
					break;
				}
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

			if(sem_wait(write_sem) == -1) {
				perror("semop lock");
				break;
			}

			fd_out = fopen(SAVE_FILE, "a");

			if(!fd_out) {
				perror("open");
				break;
			}

			printf("%d ", num);
			fprintf(fd_out, "%d ", num);
			fflush(fd_out);

			fclose(fd_out);

			if(sem_post(write_sem) == -1) {
				perror("semop unlock");
				break;
			}
			usleep(5e5);
		}
		printf("\n");
		close(fd[0]);
		wait(NULL);

        sem_close(read_sem);
		sem_unlink(SEM_READ_NAME);

        sem_close(write_sem);
		sem_unlink(SEM_WRITE_NAME);
	}
	exit(EXIT_SUCCESS);
}
