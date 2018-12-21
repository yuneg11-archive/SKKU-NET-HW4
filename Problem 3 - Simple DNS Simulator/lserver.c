#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_DNS_LEN 128
#define MAX_IP_LEN 16

int createAndBindSocket(int port) {
    struct sockaddr_in local_addr;
    int sockfd;

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    if(bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        return -1;
    }

    return sockfd;
}

int sendToClient(int sockfd, struct sockaddr_in *client_addr_p, char *data, int data_len) {
    socklen_t client_addr_len = sizeof(*client_addr_p);
    int send_size;

    if((send_size = sendto(sockfd, data, data_len, 0, (struct sockaddr*)client_addr_p, client_addr_len)) < 0) {
        perror("sendto");
        return -1;
    }

    return 0;
}

int receiveFromClient(int sockfd, struct sockaddr_in *client_addr_p, char *data, int data_len) {
    socklen_t client_addr_len = sizeof(*client_addr_p);
    int receive_size;

    if((receive_size = recvfrom(sockfd, data, data_len, 0, (struct sockaddr*)client_addr_p, &client_addr_len)) < 0) {
        perror("recvfrom");
        return -1;
    }

    return 0;
}

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

int checkDNS(char *dns_filename, char *target_ip, char *target_dns) {
    FILE *file = fopen(dns_filename, "r");
    char dns[MAX_DNS_LEN];
    char ip[MAX_IP_LEN];

    while(!feof(file)) {
        fscanf(file, "%s %s", dns, ip);
        if(strcmp(dns, target_dns) == 0) {
            strcpy(target_ip, ip);
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return -1;
}

int main(int argc, char *argv[]) {
    int client_sockfd;
    int server_sockfd;
    char *address = "127.0.0.1";
    int client_port = 8000;
    int server_port = 9000;
    struct sockaddr_in client_addr;

    char dns_request[MAX_DNS_LEN] = {};
    char dns_response[MAX_IP_LEN] = {};

    printf("Local server starts...\n");

    if((client_sockfd = createAndBindSocket(client_port)) < 0) {
        perror("createAndBindSocket");
        exit(1);
    }

    if(receiveFromClient(client_sockfd, &client_addr, dns_request, sizeof(dns_request)) < 0) {
        perror("client_dns_request");
        exit(1);
    }

    printf("Receive request: %s\n", dns_request);

    if(checkDNS("local_dns.txt", dns_response, dns_request) < 0) {
        if((server_sockfd = connectToServer(address, server_port)) < 0) {
            perror("connectToServer");
            exit(1);
        }

        printf("Send request: %s\n", dns_request);

        if(sendToServer(server_sockfd, dns_request, sizeof(dns_request)) < 0) {
            perror("server_dns_request");
            exit(1);
        }

        if(receiveFromServer(server_sockfd, dns_response, sizeof(dns_response)) < 0) {
            perror("server_dns_response");
            exit(1);
        }

        printf("Receive response: %s\n", dns_response);
    }

    printf("Send response: %s\n", dns_response);

    if(sendToClient(client_sockfd, &client_addr, dns_response, sizeof(dns_response)) < 0) {
        perror("client_dns_response");
        exit(1);
    }

    printf("Local server terminates.\n");

    close(server_sockfd);
    close(client_sockfd);
    return 0;
}