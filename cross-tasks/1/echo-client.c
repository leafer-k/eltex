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
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <linux/if_packet.h>

#define BUFFER_SIZE 2048
#define NAME_LEN 128
#define END_STR "disconnect"

static uint32_t server_addr;
static uint32_t gateway_addr;
static int server_port;
static char interf_name[NAME_LEN];
static unsigned char gateway_mac[ETH_ALEN];


char* getGatewayIP(const char* ifname) {
    FILE *fp;
    char cmd[128];
    char *ip = NULL;
    char buffer[128];

    sprintf(cmd, "ip route show dev %s | awk '/default/ {print $3}'", ifname);
    fp = popen(cmd, "r");
    if (fp) {
        if (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            ip = strdup(buffer);
        }
        pclose(fp);
    }
    return ip;
}

int getGatewayMac(const char* ifname, unsigned char* mac) {
    char* gateway_ip = getGatewayIP(ifname);
    if (!gateway_ip) {
        fprintf(stderr, "Failed to get gateway IP\n");
        return -1;
    }

    int arp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (arp_fd < 0) {
        perror("socket for ARP");
        free(gateway_ip);
        return -1;
    }

    struct arpreq arp_req;
    memset(&arp_req, 0, sizeof(arp_req));
    struct sockaddr_in *sin = (struct sockaddr_in *)&arp_req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(gateway_ip);

    strncpy(arp_req.arp_dev, ifname, IFNAMSIZ-1);
    arp_req.arp_flags = ATF_PERM | ATF_PUBL;

    if (ioctl(arp_fd, SIOCGARP, &arp_req) < 0) {
        perror("ioctl SIOCGARP");
        close(arp_fd);
        free(gateway_ip);
        return -1;
    }

    memcpy(mac, arp_req.arp_ha.sa_data, ETH_ALEN);
    close(arp_fd);
    free(gateway_ip);
    return 0;
}

uint32_t getInterfaceIP(const char *ifname) {
    int fd;
    struct ifreq ifr;
    uint32_t ip_addr = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 0;
    }

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
        ip_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
    } else {
        perror("ioctl SIOCGIFADDR");
    }

    close(fd);
    return ip_addr;
}

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

int sendData(int sock_raw, const char* data, int my_port, uint32_t my_addr, 
             const unsigned char gateway_mac[ETH_ALEN]) 
{
    char packet[ETH_FRAME_LEN];
    memset(packet, 0, sizeof(packet));

    struct ethhdr *eth = (struct ethhdr *)packet;
    

    struct ifreq ifr;
    strncpy(ifr.ifr_name, interf_name, IFNAMSIZ);
    if (ioctl(sock_raw, SIOCGIFHWADDR, &ifr) < 0) {
        perror("ioctl SIOCGIFHWADDR");
        return -1;
    }
    
    memcpy(eth->h_source, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    memcpy(eth->h_dest, gateway_mac, ETH_ALEN);
    eth->h_proto = htons(ETH_P_IP);

    struct iphdr *ip = (struct iphdr *)(packet + sizeof(struct ethhdr));
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data));
    ip->id = htons(getpid() & 0xFFFF);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->check = 0;
    ip->saddr = my_addr;
    ip->daddr = htonl(server_addr);

    ip->check = checksum((unsigned short *)ip, sizeof(struct iphdr));

    struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));
    udp->source = htons(my_port);
    udp->dest = htons(server_port);
    udp->len = htons(sizeof(struct udphdr) + strlen(data));
    udp->check = 0;

    char *payload = packet + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(payload, data, strlen(data) + 1);

    struct sockaddr_ll sa;
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_IP);
    sa.sll_ifindex = if_nametoindex(interf_name);
    sa.sll_halen = ETH_ALEN;
    memcpy(sa.sll_addr, gateway_mac, ETH_ALEN);

    if (sendto(sock_raw, packet, sizeof(struct ethhdr) + ntohs(ip->tot_len),
        0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("sendto failed");
        return -1;
    }

    return 0;
}

int process_packet(char* buffer, int size, int my_port, int sock_raw){
	struct ethhdr *eth = (struct ethhdr *)buffer;
    if (ntohs(eth->h_proto) == ETH_P_IP) {
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        if (ip->protocol == IPPROTO_UDP) {
            struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + (ip->ihl * 4));
            if (ntohs(udp->dest) == my_port && ntohs(udp->source) == server_port) {
                int data_size = ntohs(udp->len) - sizeof(struct udphdr);
                if (data_size > 0) {
                    char* data = buffer + sizeof(struct ethhdr) + (ip->ihl * 4) + sizeof(struct udphdr);
                    data[data_size] = '\0';
					printf("[Server] %s\n", data);
				}
				return 0;
            }
        }
    }
    return -1;
}

int main(int argc, char* argv[]) {
	if(argc < 5) {
		printf("Use: %s <server ip> <server port> <interface> <my port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}    

	server_port = atoi(argv[2]);
	printf("srv prt: %d\n", server_port);
	int my_port = atoi(argv[4]);
	int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	
	strncpy(interf_name, argv[3], strlen(argv[3]));
	server_addr = inet_addr(argv[1]);
	
	if (getGatewayMac(interf_name, gateway_mac) != 0) {
        perror("Failed to get gateway MAC\n");
        exit(1);
    }
	
	gateway_addr = inet_addr(getGatewayIP(interf_name));
	
	uint32_t my_addr = getInterfaceIP(interf_name);
	
	if(sock_raw == -1) {
		perror("Socket error");
		exit(1);
	}

    printf("Echo client started. Type 'exit' to shutdown\n\n");

    int pid =  fork();

	unsigned char buffer[BUFFER_SIZE];

    if(pid == 0) {
        while(1) {
			int data_size = recvfrom(sock_raw, buffer, sizeof(buffer), 0, NULL, NULL);
			if(data_size == -1) {
				perror("recvfrom");
				close(sock_raw);
				exit(EXIT_FAILURE);
			}
			process_packet(buffer, data_size, my_port, sock_raw);
        }
    } else {
		while(1) {
			fgets(buffer, sizeof(buffer), stdin);
			buffer[strcspn(buffer, "\n")] = 0;
			
			if(strcmp(buffer, "exit") == 0) {
				sendData(sock_raw, END_STR, my_port, my_addr, gateway_mac);
				break;
			}
			
			if (sendData(sock_raw, buffer, my_port, my_addr, gateway_mac) < 0) {
				perror("Send data\n");
			}
		}
        kill(pid, SIGTERM);
        if(close(sock_raw) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}
    }

    exit(EXIT_SUCCESS);
}
