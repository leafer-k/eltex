#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_NUM_COUNT 20
#define MAX_RAND_NUM 1000
#define RES_SIZE 3

#define MEM_NAME "/shmem_task13"
#define MEM_RES_NAME "/shmem_res_task13"
#define SEM_NAME "/sem_task13"

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

	int shmid, shmid_res;

	signal(SIGINT, handle_sigint);

	if((shmid = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0600)) == -1) {
		perror("shm_open 1");
		exit(EXIT_FAILURE);
	}

	if((shmid_res = shm_open(MEM_RES_NAME, O_CREAT | O_RDWR, 0600)) == -1) {
		perror("shm_open 2");
		exit(EXIT_FAILURE);
	}

	if(ftruncate(shmid, sizeof(int) * (MAX_NUM_COUNT + 1)) == -1) {
		perror("ftruncate 1");
		exit(EXIT_FAILURE);
	}

	if(ftruncate(shmid_res, sizeof(int) * (RES_SIZE + 1)) == -1) {
		perror("ftruncate 2");
		exit(EXIT_FAILURE);
	}

	int* data = mmap(NULL, (MAX_NUM_COUNT + 1) * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
	if(data == MAP_FAILED) {
		perror("data mmap");
		exit(EXIT_FAILURE);
	}

	int* data_res = mmap(NULL, (RES_SIZE + 1) * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmid_res, 0);

	if(data_res == MAP_FAILED) {
		perror("data_res mmap");
		exit(EXIT_FAILURE);
	}

    sem_t* semid;

    if((semid = sem_open(SEM_NAME, O_CREAT, 0666, 1)) == SEM_FAILED) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

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
	        if(sem_wait(semid) == -1) {
	            perror("sem_wait");
	            break;
	        }
			int get_n = data[0];
			data[0] = -1;

			if(get_n != -1) {
				printf("[Child] Reading nums\n\n");
				data_res[0] = min_data(get_n, data);
				data_res[1] = max_data(get_n, data);
			}

			if(sem_post(semid) == -1) {
	            perror("sem_post");
	            exit(EXIT_FAILURE);
	        }
			usleep(1e3);
		}
	}
	else {
//Parent
		while(run_flag) {
	        if(sem_wait(semid) == -1) {
	            perror("sem_wait");
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

	        if(sem_post(semid) == -1) {
	            perror("sem_post");
	            break;
	        }
			usleep(1e6);
		}
		wait(NULL);
		printf("\nData sets processed: %d\n", data_res[2]);

        if(sem_close(semid) == -1) {
			perror("sem_close");
		}
        if(sem_unlink(SEM_NAME) == -1) {
			perror("sem_unlink");
		}

		if(munmap(data, (MAX_NUM_COUNT + 1) * sizeof(int)) == -1) {
			perror("munmap data");
			exit(EXIT_FAILURE);
		}

		if(shm_unlink(MEM_NAME) == -1) {
			perror("shm_unlink data");
			exit(EXIT_FAILURE);
		}

		if(munmap(data_res, (RES_SIZE + 1) * sizeof(int)) == -1) {
			perror("munmap data_res");
			exit(EXIT_FAILURE);
		}

		if(shm_unlink(MEM_RES_NAME) == -1) {
			perror("shm_unlink data_res");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}
