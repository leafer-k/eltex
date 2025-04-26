#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define SERVER_IP "89.189.178.55"
#define SERVER_PORT 62222

#define MSG_BUF_LEN 1000

int main(int argc, char* argv[]) {
    int sockfd;
    int clilen, n;
    struct sockaddr_in servaddr, cliaddr;
    char buf[MSG_BUF_LEN];

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    bzero(&cliaddr, sizeof(cliaddr));

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(0);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if(bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) <  0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int pid =  fork();

    if(pid == 0) {
        while(1) {
			socklen_t servlen = sizeof(servaddr);
            if(recvfrom(sockfd, buf, MSG_BUF_LEN - 1, 0, (struct sockaddr *) &servaddr, &servlen) < 0) {
                perror("recvfrom");
                break;
            }
            printf("[Server]: %s\n", buf);
        }
    } else {
        while(1) {
	        fgets(buf, sizeof(buf), stdin);
			printf("\n");
            if(sendto(sockfd, buf, strlen(buf) + 1, 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
                perror("sendto");
                break;
            }
        }
        wait(NULL);
        close(sockfd);
    }

    exit(EXIT_SUCCESS);
}
