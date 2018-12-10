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

int readExpression(char *filename, char *expression) {
    FILE *file = fopen(filename, "r");

    fscanf(file, "%[^\n]s", expression);
    fclose(file);

    return 0;
}

int writeResult(char *filename, char *result) {
    FILE *file = fopen(filename, "w");

    fprintf(file, "%s\n", result);
    fclose(file);

    return 0;
}

int main(int argc, char *argv[]) {
    int sockfd;
    char *address;
    int port;

    char expression[1024] = "";
    char result[32] = "";

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

    readExpression("expression.txt", expression);
    printf("Expression: %s\n", expression);

    if(sendToServer(sockfd, expression, sizeof(expression)) < 0) {
        perror("expression");
        exit(1);
    }

    if(receiveFromServer(sockfd, result, sizeof(result)) < 0) {
        perror("result");
        exit(1);
    }

    writeResult("result.txt", result);
    printf("Result: %s\n", result);

    printf("Client terminates.\n");

    close(sockfd);
    return 0;
}