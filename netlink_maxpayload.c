#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>

int main() {
    int sockfd;
    int max_payload;
    socklen_t optlen = sizeof(max_payload);

    sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &max_payload, &optlen) < 0) {
        perror("getsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Max Netlink payload size: %d\n", max_payload);

    close(sockfd);

    return 0;
}
