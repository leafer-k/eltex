#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <mqueue.h>
#include <sys/timerfd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>

#define ARG_LEN 64
#define MAX_ARGS 2
#define MAX_DRIVERS 20
#define MSG_SIZE 128
#define MAX_QUEUE_MSG 10
#define BUFF_SIZE ARG_LEN * (MAX_ARGS + 1)

#define AVAL_MSG "Available"
#define BUSY_MSG "Busy"
#define STATUS_REQ "STATUS?"

#define MAIN_QUEUE_NAME "/taxi_main"
#define QUEUE_NAME_PREF "/taxi_"

static const struct mq_attr attr = {
    .mq_flags = 0,
    .mq_maxmsg = MAX_QUEUE_MSG,
    .mq_msgsize = MSG_SIZE,
    .mq_curmsgs = 0
};

static const char* command_list[] = {"create_driver", "send_task", "get_status", "get_drivers", "help", NULL};
static int drivers_pids[MAX_DRIVERS];
static mqd_t mq;

volatile sig_atomic_t running = 1;

void cleanup_and_exit(int signo) {
    running = 0;
    mq_close(mq);
    mq_unlink(MAIN_QUEUE_NAME);
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (drivers_pids[i] != -1) {
            kill(drivers_pids[i], SIGTERM);
            waitpid(drivers_pids[i], NULL, 0);
            char queue_name[ARG_LEN];
            snprintf(queue_name, ARG_LEN, "%s%d", QUEUE_NAME_PREF, drivers_pids[i]);
            mq_unlink(queue_name);
        }
    }
    printf("\nCleanup complete. Exiting.\n");
    exit(EXIT_SUCCESS);
}

int create_driver() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        char queue_name[ARG_LEN];
        snprintf(queue_name, ARG_LEN, "%s%d", QUEUE_NAME_PREF, getpid());
        
        mqd_t mq_driver = mq_open(queue_name, O_CREAT | O_RDWR, 0600, &attr);
		if (mq_driver == (mqd_t)-1) {
			perror("mq_open (child)");
			exit(EXIT_FAILURE);
		}

		int timer_fd = -1;
		int busy = 0;
		char msg[MSG_SIZE];

		while (1) {
			struct pollfd fds[2];
			fds[0].fd = mq_driver;
			fds[0].events = POLLIN;

			int nfds = 1;

			if (timer_fd != -1) {
				fds[1].fd = timer_fd;
				fds[1].events = POLLIN;
				nfds = 2;
			}

			int ret = poll(fds, nfds, -1);
			if (ret == -1) {
				perror("poll");
				break;
			}

			if (fds[0].revents & POLLIN) {
				memset(msg, 0, MSG_SIZE);
				ssize_t bytes = mq_receive(mq_driver, msg, MSG_SIZE, NULL);
				if (bytes > 0) {
					if (strncmp(msg, "TASK", 4) == 0) {
						int seconds = atoi(msg + 5);

						if (timer_fd != -1) {
							close(timer_fd);
						}

						timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
						if (timer_fd == -1) {
							perror("timerfd_create");
							continue;
						}

						struct itimerspec its = {0};
						its.it_value.tv_sec = seconds;

						if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
							perror("timerfd_settime");
							close(timer_fd);
							timer_fd = -1;
							continue;
						}

						busy = 1;
					} else if (strncmp(msg, STATUS_REQ, strlen(STATUS_REQ)) == 0) {
						char resp_queue[ARG_LEN];
						sscanf(msg, "STATUS? %s", resp_queue);

						char status_msg[MSG_SIZE];
						if (busy) {
							struct itimerspec curr;
							if (timerfd_gettime(timer_fd, &curr) == 0) {
								snprintf(status_msg, MSG_SIZE, "Busy (%ld)", curr.it_value.tv_sec);
							} else {
								snprintf(status_msg, MSG_SIZE, "Busy");
							}
						} else {
							snprintf(status_msg, MSG_SIZE, "Available");
						}

						mqd_t mq_resp = mq_open(resp_queue, O_WRONLY);
						if (mq_resp != (mqd_t)-1) {
							mq_send(mq_resp, status_msg, strlen(status_msg) + 1, 0);
							mq_close(mq_resp);
						}
					}
				}
			}

			if (nfds == 2 && (fds[1].revents & POLLIN)) {
				uint64_t expirations;
				read(timer_fd, &expirations, sizeof(expirations));
				close(timer_fd);
				timer_fd = -1;
				busy = 0;
			}
		}

        
        mq_close(mq_driver);
        mq_unlink(queue_name);
        exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (drivers_pids[i] == -1) {
            drivers_pids[i] = pid;
            printf("Driver created with PID: %d\n", pid);
            return 0;
        }
    }
    printf("Driver limit reached\n");
    return -1;
}

int send_task(int pid, int task_timer) {
    char queue_name[ARG_LEN];
    snprintf(queue_name, ARG_LEN, "%s%d", QUEUE_NAME_PREF, pid);

    char resp_queue[ARG_LEN];
    snprintf(resp_queue, ARG_LEN, "/taxi_resp_%d", getpid());
    struct mq_attr resp_attr = attr;
    resp_attr.mq_flags = O_NONBLOCK;

    mqd_t mq_resp = mq_open(resp_queue, O_CREAT | O_RDONLY | O_NONBLOCK, 0600, &resp_attr);
    if (mq_resp == (mqd_t)-1) {
        perror("mq_open resp_queue");
        return -1;
    }

    mqd_t target = mq_open(queue_name, O_WRONLY);
    if (target == (mqd_t)-1) {
        perror("mq_open (send_task)");
        mq_close(mq_resp);
        mq_unlink(resp_queue);
        return -1;
    }

    char status_msg[MSG_SIZE];
    snprintf(status_msg, MSG_SIZE, "%s %s", STATUS_REQ, resp_queue);
    if (mq_send(target, status_msg, strlen(status_msg) + 1, 0) == -1) {
        perror("mq_send (status)");
        mq_close(target);
        mq_close(mq_resp);
        mq_unlink(resp_queue);
        return -1;
    }

    struct pollfd pfd;
    pfd.fd = mq_resp;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, 300);

    char response[MSG_SIZE] = "";
    if (ret > 0 && (pfd.revents & POLLIN)) {
        ssize_t bytes = mq_receive(mq_resp, response, MSG_SIZE, NULL);
        if (bytes < 0) {
            perror("mq_receive");
        }
    } else {
        printf("Driver %d did not respond to status request.\n", pid);
    }

    mq_close(mq_resp);
    mq_unlink(resp_queue);

    if (strncmp(response, "Busy", 4) == 0) {
        printf("Driver %d is busy. Task not sent.\n", pid);
        mq_close(target);
        return -1;
    }

    char msg[MSG_SIZE];
    snprintf(msg, MSG_SIZE, "TASK %d", task_timer);
    if (mq_send(target, msg, strlen(msg) + 1, 0) == -1) {
        perror("mq_send (task)");
        mq_close(target);
        return -1;
    }
    mq_close(target);
    printf("Task sent to driver %d for %d seconds\n", pid, task_timer);
    return 0;
}


int get_status(int pid) {
    char queue_name[ARG_LEN];
    snprintf(queue_name, ARG_LEN, "%s%d", QUEUE_NAME_PREF, pid);

    mqd_t mq_driver = mq_open(queue_name, O_WRONLY);
    if (mq_driver == (mqd_t)-1) {
        perror("mq_open (get_status)");
        return -1;
    }

    char resp_queue[ARG_LEN];
    snprintf(resp_queue, ARG_LEN, "/taxi_resp_%d", getpid());
    struct mq_attr resp_attr = attr;
    resp_attr.mq_flags = O_NONBLOCK;

    mqd_t mq_resp = mq_open(resp_queue, O_CREAT | O_RDONLY | O_NONBLOCK, 0600, &resp_attr);
    if (mq_resp == (mqd_t)-1) {
        perror("mq_open resp_queue");
        mq_close(mq_driver);
        return -1;
    }

    char msg[MSG_SIZE];
    snprintf(msg, MSG_SIZE, "%s %s", STATUS_REQ, resp_queue);
    if (mq_send(mq_driver, msg, strlen(msg) + 1, 0) == -1) {
        perror("mq_send (status)");
        mq_close(mq_driver);
        mq_close(mq_resp);
        mq_unlink(resp_queue);
        return -1;
    }

    struct pollfd pfd;
    pfd.fd = mq_resp;
    pfd.events = POLLIN;

    char response[MSG_SIZE];
    int ret = poll(&pfd, 1, 300);

    if (ret > 0 && (pfd.revents & POLLIN)) {
        ssize_t bytes = mq_receive(mq_resp, response, MSG_SIZE, NULL);
        if (bytes >= 0) {
            printf("Driver %d status: %s\n", pid, response);
        } else {
            perror("mq_receive (status)");
        }
    } else {
        printf("Driver %d status: No response (timeout)\n", pid);
    }

    mq_close(mq_driver);
    mq_close(mq_resp);
    mq_unlink(resp_queue);
    return 0;
}


int get_drivers() {
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (drivers_pids[i] != -1) {
            printf("Driver PID: %d\n", drivers_pids[i]);
        }
    }
    return 0;
}

int help() {
    printf("Commands:\n\tcreate_driver\n\tsend_task <pid> <task_timer>\n\tget_status <pid>\n\tget_drivers\n\thelp\n\n");
    return 0;
}

int parseCommand(char* in) {
    int args_cnt = 0;
    int command_num = -1;
    int total_commands = 0;
    char args[MAX_ARGS][ARG_LEN];
    memset(args, 0, sizeof(args));
    int args_start = 0;
    for (; command_list[total_commands] != NULL; total_commands++);
    for (int i = 0; i < strlen(in); i++) {
        if (in[i] == ' ') {
            args_cnt++;
            args_start = args_start == 0 ? i + 1 : args_start;
        }
    }
    if (args_cnt > MAX_ARGS) {
        printf("Too many arguments\n");
        return -1;
    }
    int j = 0, k = 0;
    for (int i = args_start; i < strlen(in); i++) {
        if (in[i] == ' ') {
            args[j][k] = '\0'; j++; k = 0; continue;
        }
        args[j][k] = in[i]; k++;
    }
    args[j][k] = '\0';
    for (int i = 0; i < total_commands; i++) {
        if (strncmp(in, command_list[i], strlen(command_list[i])) == 0 && (in[strlen(command_list[i])] == ' ' || in[strlen(command_list[i])] == '\0')) {
            command_num = i;
        }
    }
    if (command_num == -1) {
        printf("Invalid command\n");
        return -1;
    }

    switch (command_num) {
        case 0: return create_driver();
        case 1: return send_task(atoi(args[0]), atoi(args[1]));
        case 2: return get_status(atoi(args[0]));
        case 3: return get_drivers();
        case 4: return help();
        default: break;
    }
    return -1;
}

int main(int argc, char* argv[]) {
    char buffer[BUFF_SIZE];
    for (int i = 0; i < MAX_DRIVERS; i++) drivers_pids[i] = -1;
    struct sigaction sa;
    sa.sa_handler = cleanup_and_exit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    mq = mq_open(MAIN_QUEUE_NAME, O_CREAT | O_RDWR, 0600, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    while (running) {
        printf(">> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;
        parseCommand(buffer);
    }
    cleanup_and_exit(0);
    return 0;
}
