#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define CLIENT_CONN_WAIT_QUEUE_SIZE 3

int listenAndAcceptClient(int port) {
    struct sockaddr_in local_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int listen_sockfd;
    int sockfd;

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((listen_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    if(bind(listen_sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        return -1;
    }

    if(listen(listen_sockfd, CLIENT_CONN_WAIT_QUEUE_SIZE) < 0) {
        perror("listen");
        return -1;
    }

    if((sockfd = accept(listen_sockfd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
        perror("accept");
        return -1;
    }

    close(listen_sockfd);
    return sockfd;
}

int sendToClient(int sockfd, char *data, int data_len) {
    int total_send_size = 0;
    int send_size;

    do {
        send_size = send(sockfd, &data[total_send_size], data_len - total_send_size, 0);
        total_send_size = send_size;
        if(send_size < 0) {
            perror("send");
            close(sockfd);
            return -1;
        }
    } while(total_send_size < data_len);

    return 0;
}

int receiveFromClient(int sockfd, char *data, int data_len) {
    int total_receive_size = 0;
    int receive_size;

    do {
        receive_size = recv(sockfd, &data[total_receive_size], data_len - total_receive_size, 0);
        total_receive_size = receive_size;
        if(receive_size < 0) {
            perror("recv");
            close(sockfd);
            return -1;
        }
    } while(total_receive_size < data_len);

    return 0;
}

void moveDrone(char *move, char *position) {
    static int x = 5;
    static int y = 5;

    if(strcasecmp(move, "Move_UP") == 0) {
        if(y > 0) y--;
    } else if(strcasecmp(move, "Move_DOWN") == 0) {
        if(y < 10) y++;
    } else if(strcasecmp(move, "Move_LEFT") == 0) {
        if(x > 0) x--;
    } else if(strcasecmp(move, "Move_RIGHT") == 0) {
        if(x < 10) x++;
    }

    sprintf(position, "(%d,%d)", x, y);
}

int main(int argc, char *argv[]) {
    int sockfd;
    int port;

    char move[16] = {};
    char position[16] = {};

    if(argc != 2) {
        printf("Usage: ./server [port]\n");
        exit(1);
    }

    printf("Server starts...\n");

    port = atoi(argv[1]);

    if((sockfd = listenAndAcceptClient(port)) < 0) {
        perror("listenAndAcceptClient");
        exit(1);
    }

    while(1) {
        if(receiveFromClient(sockfd, move, sizeof(move)) < 0) {
            perror("move");
            exit(1);
        }

        if(strcmp(move, "exit") == 0)
            break;

        printf("Move: %s\n", move);
        moveDrone(move, position);
        printf("Position: %s\n", position);

        if(sendToClient(sockfd, position, sizeof(position)) < 0) {
            perror("position");
            exit(1);
        }
    }

    printf("Server terminates.\n");

    close(sockfd);
    return 0;
}