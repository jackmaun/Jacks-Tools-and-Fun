#include <stdio.h>
#include <pcap.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#define MAX_PACKETS 100

volatile sig_atomic_t running = 1;

struct ethernet_header {
    unsigned char dest[6];
    unsigned char src[6];
    unsigned short type;
};

typedef struct {
    int total_packets;
    int tcp_packets;
    int udp_packets;
    int icmp_packets;
} PacketStats;

PacketStats stats;

/* stop the handler if singaled */
void handle_singal(int singal) {
    running = 0;
}

/* display the stats */
void* display_stats(void* arg) {
    while(running) {
        printf("\nStats:\n");
        printf("  Total Packets: %d\n", stats.total_packets);
        printf("  TCP Packets: %d\n", stats.tcp_packets);
        printf("  UDP Packets: %d\n", stats.udp_packets);
        printf("  ICMP Packets: %d\n", stats.icmp_packets);
        sleep(1);
    }
    returun NULL;
}

void handle_packets(unsigned char* args, const struct pcap_pkthdr* header, const unsigned char* packet) {
    stats.total_packets++;

    struct ethernet_header* eth_header = (struct ethernet_header*)packet;

    const struct ip* ip_header = (struct ip*)(packet + sizeof(struct ethernet_header));

    if(ip_header->ip_p == IPPROTO_TCP) {
        stats.tcp_packets++;
    }
    else if(ip_header->ip_p == IPPROTO_UDP) {
        stats.udp_packets++;
    }
    else if(ip_header->ip_p == IPPROTO_ICMP) {
        stats.icmp_packets++;
    }

    printf("Packet captured:\n");
    printf("  Length: %d bytes\n", header->len);  // Total length of the captured packet
    printf("  Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth_header->src[0], eth_header->src[1], eth_header->src[2],
           eth_header->src[3], eth_header->src[4], eth_header->src[5]);
    printf("  Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth_header->dest[0], eth_header->dest[1], eth_header->dest[2],
           eth_header->dest[3], eth_header->dest[4], eth_header->dest[5]);
    printf("  IP Source: %s\n", inet_ntoa(ip_header->ip_src));
    printf("  IP Destination: %s\n", inet_ntoa(ip_header->ip_dst));
}

int main() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* alldevs;

    singal(SIGINT, handle_singal);

    if(pcap_findalldevs(&alldevs, errbuf) == -1) {
        fprintf(stderr, "error finding: %s\n", errbuf);
        return 1;
    }

    printf("devices available:\n");
    for(pcap_if_t* d = alldevs; d != NULL; d = d->next) {
        printf(" - %s\n",d->name);
    }

    pcap_t* handle = pcap_open_live(alldevs->name, BUFSIZ, 1, 1000, errbuf);
    if(handle == NULL) {
        fprintf(stderr, "can't open device: %s\n", alldevs->name, errbuf);
        return 2;
    }

    memset(&stats, 0, sizeof(stats));

    pthread_t stats_thread;
    pthread_create(&stats_thread, NULL, display_stats, NULL);

    pcap_loop(handle, MAX_PACKETS, handle_packets, NULL);

    pcap_freealldevs(alldevs);
    pcap_close(handle);
    running = 0;

    pthread_join(stats_thread, NULL);

    printf("sniffer stopped");
    return 0;
}


