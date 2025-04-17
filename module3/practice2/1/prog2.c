#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>

#define MSG_SIZE 256
#define EXIT_PRIORITY 255

struct msgbuf {
	long mtype;
	char mtext[MSG_SIZE];
};


int main(int argc, char* argv[]) {
	key_t key = ftok(".", 1);
	int msgid = msgget(key, 0666 | IPC_CREAT);

	if(msgid == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	printf("Chat 2 started, send 'exit' to quit\n");

	while(1) {
		struct msgbuf send_msg;
		send_msg.mtype = 2;
		fgets(send_msg.mtext, MSG_SIZE, stdin);

		if (msgsnd(msgid, &send_msg, sizeof(send_msg.mtext), 0) == -1) {
			perror("msgsnd");
			break;
		}

		if (strncmp(send_msg.mtext, "exit", 4) == 0) {
			send_msg.mtype = EXIT_PRIORITY;
			msgsnd(msgid, &send_msg, sizeof(send_msg.mtext), 0);
			printf("Closing\n");
			break;
		}

		struct msgbuf recv_msg;

		if(msgrcv(msgid, &recv_msg, sizeof(recv_msg.mtext), 1, 0) == -1) {
			perror("msgrcv");
			break;
		}

		printf("> %s", recv_msg.mtext);
		if(strncmp(recv_msg.mtext, "exit", 4) == 0) {
			printf("Closing\n");
			break;
		}

	}

	exit(EXIT_SUCCESS);
}
