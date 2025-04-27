#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define HOST_ADDR "89.189.178.55"
#define PORT 62222
#define MAX_CONNECTIONS 10

#define MSG_BUF_LEN 1000

int main(int argc, char* argv[]) {
	int sockfd;
	int shmid;
	int clilen = 0, n;
	struct sockaddr_in servaddr;
	char buf[MSG_BUF_LEN];
	char buf_sendMsg[MSG_BUF_LEN];

	key_t shm_key = ftok(".", 1);

	if(shm_key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}

    if((shmid = shmget(shm_key, sizeof(struct sockaddr_in), IPC_CREAT | 0600)) == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

	struct sockaddr_in *cliaddr = shmat(shmid, NULL, 0);

	if(cliaddr == (struct sockaddr_in *) -1) {
		perror("shmat");
		exit(EXIT_FAILURE);
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) <  0) {
		perror("bind");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	int pid =  fork();

	if(pid == 0) {
		int was_connected = 0;
		while(1) {
			clilen = sizeof(cliaddr);
			if(recvfrom(sockfd, buf, MSG_BUF_LEN - 1, 0, (struct sockaddr *) cliaddr, &clilen) < 0) {
				perror("recvfrom");
				break;
			}

			char* cli_ip = inet_ntoa(cliaddr->sin_addr);

			if(was_connected == 0) {
				printf("%s:%d Connected!\n\n", cli_ip, ntohs(cliaddr->sin_port));
				was_connected = 1;
			}

			if(strcmp(buf, "disconnect") == 0) {
				cliaddr->sin_port = 0;
				printf("%s disconnected!\n\n", cli_ip);
				was_connected = 0;
			} else {
				printf("[%s:%d]: %s\n", cli_ip, ntohs(cliaddr->sin_port), buf);
			}
		}
char* cli_addr = inet_ntoa(cliaddr->sin_addr);	} else {
		while(1) {
			fgets(buf_sendMsg, sizeof(buf_sendMsg), stdin);
			if(strcmp(buf_sendMsg, "exit\n") == 0) {
				break;
			}
			printf("\n");
			if(cliaddr->sin_port != 0) {
				if(sendto(sockfd, buf_sendMsg, strlen(buf_sendMsg) + 1, 0, (struct sockaddr *) cliaddr, sizeof(*cliaddr)) < 0) {
					perror("sendto");
					break;
				}
			} else {
				printf("Client not connected\n");
			}
		}
		kill(pid, SIGTERM);

		if(close(sockfd) == -1) {
			perror("shmdt");
		}

        if(shmdt(cliaddr) == -1) {
        	perror("shmdt");
        }

        if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        	perror("shmctl IPC_RMID");
        }

	}
	exit(EXIT_SUCCESS);
}
