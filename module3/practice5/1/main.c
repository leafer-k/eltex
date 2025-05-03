#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define DUMP_FILENAME "dump.bin"


void saveToFile(char* data, int size) {
    int fd = open(DUMP_FILENAME, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1) {
        perror("open");
        return;
    }

    int bytes_written = write(fd, data, size);
    if (bytes_written == -1) {
        perror("write");
    }
    close(fd);
}

void processPacket(char *buffer, int size, int server_port) {
    struct ethhdr *eth = (struct ethhdr *)buffer;
    if (ntohs(eth->h_proto) == ETH_P_IP) {
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        if (ip->protocol == IPPROTO_UDP) {
            struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + (ip->ihl * 4));
            if (ntohs(udp->dest) == server_port) {
                int data_size = ntohs(udp->len) - sizeof(struct udphdr);
                if (data_size > 0) {
                    char* data = buffer + sizeof(struct ethhdr) + (ip->ihl * 4) + sizeof(struct udphdr);
                    printf("Captured %d bytes\n", data_size);
                    saveToFile(data, data_size);
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
        printf("Use: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	int fd = open(DUMP_FILENAME, O_WRONLY | O_CREAT, 0600);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
	close(fd);

	int server_port = atoi(argv[1]);

	int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (sock_raw == -1) {
    	perror("Socket error");
    	exit(1);
	}

    unsigned char buffer[65536];
    while (1) {
        int data_size = recvfrom(sock_raw, buffer, sizeof(buffer), 0, NULL, NULL);
        if (data_size == -1) {
            perror("recvfrom");
            close(sock_raw);
            exit(EXIT_FAILURE);
        }
		processPacket(buffer, data_size, server_port);
    }

	close(sock_raw);
	exit(EXIT_SUCCESS);
}
