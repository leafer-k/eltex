#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>
#include <mqueue.h>

#define MSG_SIZE 256
#define QUEUE_NAME "/chat2-2"

int main(int argc, char* argv[]) {
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = MSG_SIZE,
        .mq_curmsgs = 0
    };

	mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);
	int prior;

	if (mq == (mqd_t)-1) {
		perror("mq_open");
		exit(EXIT_FAILURE);
	}

	printf("Chat started, send 'exit' to quit\n");

	while(1) {
		char recv_msg[MSG_SIZE];

		if(mq_receive(mq, recv_msg, MSG_SIZE, &prior) == -1) {
			perror("mq_recieve");
			break;
		}

		if(prior == 2) {
			printf("> %s", recv_msg);
		}

		if(strncmp(recv_msg, "exit", 4) == 0) {
			printf("Closing\n");
			break;
		}

		char send_msg[MSG_SIZE];
		fgets(send_msg, MSG_SIZE, stdin);

		if (mq_send(mq, &send_msg, MSG_SIZE, 1) == -1) {
			perror("mq_send");
			break;
		}

		if (strncmp(send_msg, "exit", 4) == 0) {
			printf("Closing\n");
			break;
		}
	}

	mq_close(mq);
	mq_unlink(QUEUE_NAME);

	exit(EXIT_SUCCESS);
}
