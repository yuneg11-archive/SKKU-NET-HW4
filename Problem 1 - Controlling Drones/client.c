#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int connectToServer(char *addr, int port) {
    struct sockaddr_in serv_addr;
    int sockfd;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(addr);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return -1;
    }

    return sockfd;
}

int sendToServer(int sockfd, char *data, int data_len) {
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

int receiveFromServer(int sockfd, char *data, int data_len) {
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

int readMove(char *filename, char *move) {
    static FILE *file = NULL;
    
    if(file == NULL)
        file = fopen(filename, "r");

    if(feof(file)) {
        fclose(file);
        return -1;
    } else {
        if(fscanf(file, " %s", move) < 0)
            return -1;
    }

    return 0;
}

int writePosition(char *filename, char *position) {
    FILE *file = fopen(filename, "w");

    fprintf(file, "%s\n", position);
    fclose(file);

    return 0;
}

int main(int argc, char *argv[]) {
    int sockfd;
    char *address;
    int port;

    char move[16] = "";
    char position[16] = "";

    if(argc != 3) {
        printf("Usage: ./client [server_ip] [port]\n");
        exit(1);
    }

    printf("Client starts...\n");

    address = argv[1];
    port = atoi(argv[2]);

    if((sockfd = connectToServer(address, port)) < 0) {
        perror("connectToServer");
        exit(1);
    }

    while(readMove("moves.txt", move) != -1) {
        printf("Move: %s\n", move);

        if(sendToServer(sockfd, move, sizeof(move)) < 0) {
            perror("move");
            exit(1);
        }

        if(receiveFromServer(sockfd, position, sizeof(position)) < 0) {
            perror("position");
            exit(1);
        }

        writePosition("position.txt", position);
        printf("Position: %s\n", position);
    }

    if(sendToServer(sockfd, "exit", sizeof(move)) < 0) {
        perror("exit");
        exit(1);
    }

    printf("Client terminates.\n");

    close(sockfd);
    return 0;
}