#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <linux/if_packet.h>

#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 2048
#define IP_BUFF_SIZE 16

#define END_STR "disconnect"

//My interface: "enp4s0"

struct client_data {
	int port;
	uint32_t addr;
	int msg_count;
};

static struct client_data* clients[MAX_CONNECTIONS];
static uint32_t my_addr;
static char eth_name[128];

const unsigned char gateway_mac[ETH_ALEN] = {0x30, 0x5a, 0x3a, 0x6e, 0xbf, 0xa8};  

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

void ip_itos(char* buff, uint32_t addr) {
	int frag[4];
	uint32_t mask = 0b00000000000000000000000011111111;
	for(int i = 0; i < 4; i++) {
		frag[i] = addr & mask;
		addr = addr >> 8;
	}
	sprintf(buff, "%d.%d.%d.%d", frag[3], frag[2], frag[1], frag[0]);
}

int isClientEmpty(struct client_data* cli) {
	if (cli->addr == 0) return 1;
	return 0;
}

int findCliByInfo(struct client_data* arr[], uint32_t addr, int port) {
	for(int i = 0; i < MAX_CONNECTIONS; i++) {
		if(!isClientEmpty(arr[i])) {
			if (arr[i]->addr == addr && arr[i]->port == port) return i;
		}
	}
	return -1;
}



int delCliByIndex(struct client_data* arr[], int n) {
	if(n < 0) return -1;
	if(isClientEmpty(arr[n])) return -1;
	arr[n]->addr = 0;
	arr[n]->port = 0;
	arr[n]->msg_count = 0;
	return 0;
}

int addCliInfo(struct client_data* arr[], uint32_t addr, int port){
	int i = 0;
	for(;i < MAX_CONNECTIONS; i++) {
		if(isClientEmpty(arr[i])) {
			arr[i]->addr = addr;
			arr[i]->port = port;
			arr[i]->msg_count = 0;
			return i;
		}
	}
	return -1;
}

int sendData(int sock_raw, struct client_data* cli, char* rcv_data, int server_port) {
    char data[BUFFER_SIZE + 10];
    rcv_data[strlen(rcv_data)-1] = '\0';
    sprintf(data, "%s %d", rcv_data, cli->msg_count);
    
    char packet[ETH_FRAME_LEN];
    memset(packet, 0, sizeof(packet));

    struct ethhdr *eth = (struct ethhdr *)packet;
    memset(eth->h_dest, 0xFF, ETH_ALEN);
    memset(eth->h_source, 0x00, ETH_ALEN);
    eth->h_proto = htons(ETH_P_IP);
    
    struct ifreq ifr;
	strncpy(ifr.ifr_name, eth_name, IFNAMSIZ);
	ioctl(sock_raw, SIOCGIFHWADDR, &ifr);
    memcpy(eth->h_source, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	memcpy(eth->h_dest, gateway_mac, ETH_ALEN);

    struct iphdr *ip = (struct iphdr *)(packet + sizeof(struct ethhdr));
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data));
    ip->id = htons(server_port);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->check = 0;
    ip->saddr = my_addr;
    ip->daddr = htonl(cli->addr);

    struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));
    udp->source = htons(server_port);
    udp->dest = htons(cli->port);
    udp->len = htons(sizeof(struct udphdr) + strlen(data));
    udp->check = 0;

    char *payload = packet + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(payload, data, strlen(data));

    ip->check = checksum((unsigned short *)ip, sizeof(struct iphdr));

    struct sockaddr_ll sa;
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_IP);
    sa.sll_ifindex = if_nametoindex(eth_name);
    sa.sll_halen = ETH_ALEN;
    memset(sa.sll_addr, 0xFF, ETH_ALEN);

    if (sendto(sock_raw, packet, 
              sizeof(struct ethhdr) + ntohs(ip->tot_len),
              0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("sendto failed");
        return -1;
    }

    return 0;
}

int process_packet(char* buffer, int size, int server_port, int sock_raw){
	struct ethhdr *eth = (struct ethhdr *)buffer;
    if (ntohs(eth->h_proto) == ETH_P_IP) {
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        if (ip->protocol == IPPROTO_UDP) {
            struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + (ip->ihl * 4));
            if (ntohs(udp->dest) == server_port) {
                int data_size = ntohs(udp->len) - sizeof(struct udphdr);
                if (data_size > 0) {
                    char* data = buffer + sizeof(struct ethhdr) + (ip->ihl * 4) + sizeof(struct udphdr);
                    char ip_str[IP_BUFF_SIZE];
                    ip_itos(ip_str, ntohl(ip->saddr));
                    int index = findCliByInfo(clients, ntohl(ip->saddr), ntohs(udp->source));
                    if(index == -1) {
						printf("[%s:%d] Connected!\n", ip_str, ntohs(udp->source));
						index = addCliInfo(clients, ntohl(ip->saddr), ntohs(udp->source));
						my_addr = ip->daddr;
						if(index == -1) {
							printf("[%s:%d] Unable to connect!", ip_str, ntohs(udp->source));
							return 0;
						}
					}
					
                    if(strncmp(data, END_STR, strlen(END_STR)) == 0) {
						if(index != -1) {
							printf("[%s:%d] Disconnected!\nTotal recieved %d messages\n", ip_str, ntohs(udp->source), clients[index]->msg_count);
							delCliByIndex(clients, index);
						}
					} else {
						clients[index]->msg_count++;
						printf("Recieved %d bytes from %s:%d (%d)\n", data_size, ip_str, ntohs(udp->source), clients[index]->msg_count);
						
						sendData(sock_raw, clients[index], data, server_port);
					}
					return 0;
                }
            }
        }
    }
    return -1;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
        printf("Use: %s <port> <interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }    
    
    strcpy(eth_name, argv[2]);
    
    printf("Server started\n");
    
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
		clients[i] = malloc(sizeof(struct client_data));
        if (clients[i] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        
		clients[i]->addr = 0;
		clients[i]->port = 0;
		clients[i]->msg_count = 0;
	}

	int server_port = atoi(argv[1]);
	int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	
	if(sock_raw == -1) {
		perror("Socket error");
		exit(1);
	}
	
	unsigned char buffer[BUFFER_SIZE];
	
	while(1){
		int data_size = recvfrom(sock_raw, buffer, sizeof(buffer), 0, NULL, NULL);
		if(data_size == -1) {
			perror("recvfrom");
			close(sock_raw);
			exit(EXIT_FAILURE);
		}
		process_packet(buffer, data_size, server_port, sock_raw);
	}
	
	
}
