#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    char ifname[] = "wlp2s0f0";  
    struct ifreq if_idx;
    struct ifreq if_mac;
    struct sockaddr_ll sa;
    char buffer[ETH_FRAME_LEN];  

    
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifname, IFNAMSIZ - 1);
    ioctl(sockfd, SIOCGIFINDEX, &if_idx);

    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifname, IFNAMSIZ - 1);
    ioctl(sockfd, SIOCGIFHWADDR, &if_mac);

    memset(buffer, 0, ETH_FRAME_LEN);
    unsigned char *etherhead = (unsigned char *) buffer;

    etherhead[0] = 0xff;
    etherhead[1] = 0xff;
    etherhead[2] = 0xff;
    etherhead[3] = 0xff;
    etherhead[4] = 0xff;
    etherhead[5] = 0xff;

    // 源 MAC
    memcpy(etherhead + 6, if_mac.ifr_hwaddr.sa_data, 6);

    // 以太网类型字段（自定义，0x88B5 为示例）
    etherhead[12] = 0x88;
    etherhead[13] = 0xB5;

    // 数据负载（从第14字节开始）
    const char *payload = "hello_mac_layer";
    memcpy(buffer + 14, payload, strlen(payload));

    // 设置发送地址
    memset(&sa, 0, sizeof(struct sockaddr_ll));
    sa.sll_ifindex = if_idx.ifr_ifindex;
    sa.sll_halen = ETH_ALEN;
    memcpy(sa.sll_addr, etherhead, 6);

    // 发送以太网帧
    int sent = sendto(sockfd, buffer, 14 + strlen(payload), 0,
                      (struct sockaddr*)&sa, sizeof(struct sockaddr_ll));
    if (sent < 0) {
        perror("sendto");
    }

    close(sockfd);
    return 0;
}