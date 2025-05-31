#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <linux/if_packet.h>

#define BUFFER_SIZE 2048
#define SERVER_IP "89.189.178.55"
#define SERVER_PORT 62222
#define INTERFACE "enp4s0"
#define END_STR "disconnect"

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

int sendRawPacket(int sock_raw, uint32_t dest_ip, int dest_port, char *data) {
    char packet[ETH_FRAME_LEN];
    memset(packet, 0, sizeof(packet));

    struct ethhdr *eth = (struct ethhdr *)packet;
    memset(eth->h_dest, 0xFF, ETH_ALEN); // Broadcast MAC (замените на MAC шлюза)
    memset(eth->h_source, 0x00, ETH_ALEN); // Заполнится ниже
    eth->h_proto = htons(ETH_P_IP);

    struct iphdr *ip = (struct iphdr *)(packet + sizeof(struct ethhdr));
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data));
    ip->id = htons(getpid());
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->check = 0;
    ip->saddr = INADDR_ANY; // Будет заполнено ядром
    ip->daddr = dest_ip;

    struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));
    udp->source = htons(getpid()); // Временный порт
    udp->dest = htons(dest_port);
    udp->len = htons(sizeof(struct udphdr) + strlen(data));
    udp->check = 0;

    char *payload = packet + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(payload, data, strlen(data));

    ip->check = checksum((unsigned short *)ip, sizeof(struct iphdr));

    struct sockaddr_ll sa;
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_IP);
    sa.sll_ifindex = if_nametoindex(INTERFACE);
    sa.sll_halen = ETH_ALEN;
    memset(sa.sll_addr, 0xFF, ETH_ALEN);

    return sendto(sock_raw, packet, sizeof(struct ethhdr) + ntohs(ip->tot_len),
                 0, (struct sockaddr *)&sa, sizeof(sa));
}

int processPacket(char *buffer, int size, int expected_port) {
    struct ethhdr *eth = (struct ethhdr *)buffer;
    if (ntohs(eth->h_proto) == ETH_P_IP) {
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        if (ip->protocol == IPPROTO_UDP) {
            struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + (ip->ihl * 4));
            if (ntohs(udp->dest) == getpid()) { // Проверяем наш временный порт
                int data_size = ntohs(udp->len) - sizeof(struct udphdr);
                if (data_size > 0) {
                    char *data = buffer + sizeof(struct ethhdr) + (ip->ihl * 4) + sizeof(struct udphdr);
                    data[data_size] = '\0';
                    printf("[Server]: %s\n", data);
                    return 1;
                }
            }
        }
    }
    return 0;
}

int main() {
    int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_raw < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("Client started. Type 'exit' to shutdown <<<\n\n");

    int pid = fork();
    if (pid == 0) {
        // Child process - receiving
        unsigned char buffer[BUFFER_SIZE];
        while (1) {
            int data_size = recvfrom(sock_raw, buffer, sizeof(buffer), 0, NULL, NULL);
            if (data_size < 0) {
                perror("recvfrom");
                break;
            }
            processPacket(buffer, data_size, getpid());
        }
    } else {
        // Parent process - sending
        uint32_t server_ip = inet_addr(SERVER_IP);
        char buf[BUFFER_SIZE];
        
        while (1) {
            fgets(buf, sizeof(buf), stdin);
            if (strcmp(buf, "exit\n") == 0) {
                sendRawPacket(sock_raw, server_ip, SERVER_PORT, END_STR);
                break;
            }
            sendRawPacket(sock_raw, server_ip, SERVER_PORT, buf);
        }
        
        kill(pid, SIGTERM);
        close(sock_raw);
    }

    return 0;
}
