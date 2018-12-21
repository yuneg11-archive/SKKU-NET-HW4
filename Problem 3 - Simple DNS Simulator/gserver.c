#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define CLIENT_CONN_WAIT_QUEUE_SIZE 3
#define MAX_DNS_LEN 128
#define MAX_IP_LEN 16

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
    int sockfd;
    int port;

    char dns_request[MAX_DNS_LEN] = {};
    char dns_response[MAX_IP_LEN] = {};

    printf("Global server starts...\n");

    port = atoi("9000");

    if((sockfd = listenAndAcceptClient(port)) < 0) {
        perror("listenAndAcceptClient");
        exit(1);
    }

    if(receiveFromClient(sockfd, dns_request, sizeof(dns_request)) < 0) {
        perror("dns_request");
        exit(1);
    }

    printf("Request: %s\n", dns_request);

    if(checkDNS("global_edu_dns.txt", dns_response, dns_request) < 0) {
        if(checkDNS("global_com_dns.txt", dns_response, dns_request) < 0) {
            strcpy(dns_response, "Not found");
        }
    }

    printf("Response: %s\n", dns_response);

    if(sendToClient(sockfd, dns_response, sizeof(dns_response)) < 0) {
        perror("dns_response");
        exit(1);
    }

    printf("Global server terminates.\n");

    close(sockfd);
    return 0;
}