#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_NUM_COUNT 20
#define MAX_RAND_NUM 1000

volatile sig_atomic_t run_flag = 1;

void handle_sigint(int sig) {
	run_flag = 0;
}

int max_data (int n, int* dataptr) {
	int max = -1;
	for(int i = 1; i < n+1; i++) {
		max = max > dataptr[i] ? max : dataptr[i];
	}
	return max;
}

int min_data (int n, int* dataptr) {
	int min = MAX_RAND_NUM;
	for(int i = 1; i < n+1; i++) {
		min = min < dataptr[i] ? min : dataptr[i];
	}
	return min;
}

int main(int argc, char* argv[]) {
	srand(time(NULL));
	key_t key1 = ftok(".", 1);
	key_t key2 = ftok(".", 2);
	key_t key3 = ftok(".", 3);

	if (key1 == -1 || key2 == -1 || key3 == -1) {
	    perror("ftok");
	    exit(EXIT_FAILURE);
	}

	int shmid, shmid_res;

	signal(SIGINT, handle_sigint);

	if((shmid = shmget(key1, (MAX_NUM_COUNT + 1) * sizeof(int), IPC_CREAT | 0600)) == -1) {
		perror("shmget");
		exit(EXIT_FAILURE);
	}

	if((shmid_res = shmget(key2, 4 * sizeof(int), IPC_CREAT | 0600)) == -1) {
		perror("shmget");
		exit(EXIT_FAILURE);
	}

	int* data = shmat(shmid, NULL, 0);
	int* data_res = shmat(shmid_res, NULL, 0);

	if(data == (int*) -1 || data_res == (int*) -1) {
		perror("shmat");
		exit(EXIT_FAILURE);
	}

	int semid;

    if((semid = semget(key3, 1, IPC_CREAT | 0660)) == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    if(semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    struct sembuf lock = {0, -1, 0};
    struct sembuf unlock[2] = {{0, 0, 0}, {0, 1, 0}};

	data[0] = -1;
	data_res[0] = -1;
	data_res[2] = 0;
	pid_t pid = fork();

	if(pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if(pid == 0) {
//Child
		while(run_flag) {
	        if(semop(semid, &lock, 1) == -1) {
	            perror("semop lock");
	            exit(EXIT_FAILURE);
	        }
			int get_n = data[0];
			data[0] = -1;
			if(get_n != -1) {
				printf("[Child] Reading nums\n\n");
				data_res[0] = min_data(get_n, data);
				data_res[1] = max_data(get_n, data);
			}
		        if(semop(semid, unlock, 2) == -1) {
	            perror("semop unlock");
	            exit(EXIT_FAILURE);
	        }
			usleep(1e3);
		}
	}
	else {
//Parent
		while(run_flag) {
	        if(semop(semid, &lock, 1) == -1) {
	            perror("semop lock");
	            break;
	        }

			int num_count = (rand() % MAX_NUM_COUNT) + 1;
			int num;
			if(data_res[0] != -1) {
				printf("[Parent] Reults:\nMin: %d\nMax: %d\n\n", data_res[0], data_res[1]);
				data_res[0] = -1;
				data_res[2]++;
			}

			printf("[Parent] Generating %d numbers:\n", num_count);
			data[0] = num_count;
			for(int i = 1; i < num_count + 1; i++) {
				num = rand() % MAX_RAND_NUM;
				printf("%d ", num);
				data[i] = num;
			}
			printf("\n\n");

	        if(semop(semid, unlock, 2) == -1) {
	            perror("semop unlock");
	            break;
	        }
			usleep(1e6);
		}
		wait(NULL);
		printf("\nData sets processed: %d\n", data_res[2]);

		if (semctl(semid, 0, IPC_RMID) == -1) {
	        perror("semctl IPC_RMID");
			exit(EXIT_FAILURE);
	    }

		if(shmdt(data) == -1 || shmdt(data_res) == -1) {
			perror("shmdt");
			exit(EXIT_FAILURE);
		}

	    if(shmctl(shmid, IPC_RMID, NULL) == -1 || shmctl(shmid_res, IPC_RMID, NULL) == -1) {
			perror("shmctl IPC_RMID");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}
