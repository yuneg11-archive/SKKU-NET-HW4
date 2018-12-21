#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_DNS_LEN 128
#define MAX_IP_LEN 16

int createAndSetSocket(char *addr, int port, struct sockaddr_in *server_addr_p) {
    int sockfd;

    memset(server_addr_p, 0, sizeof(*server_addr_p));
    server_addr_p->sin_family = AF_INET;
    server_addr_p->sin_port = htons(port);
    server_addr_p->sin_addr.s_addr = inet_addr(addr);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    return sockfd;
}

int sendToServer(int sockfd, struct sockaddr_in *server_addr_p, char *data, int data_len) {
    socklen_t server_addr_len = sizeof(*server_addr_p);
    int send_size;

    if((send_size = sendto(sockfd, data, data_len, 0, (struct sockaddr*)server_addr_p, server_addr_len)) < 0) {
        perror("sendto");
        return -1;
    }

    return 0;
}

int receiveFromServer(int sockfd, struct sockaddr_in *server_addr_p, char *data, int data_len) {
    socklen_t server_addr_len = sizeof(*server_addr_p);
    int receive_size;
    
    if((receive_size = recvfrom(sockfd, data, data_len, 0, (struct sockaddr*)server_addr_p, &server_addr_len)) < 0) {
        perror("recvfrom");
        return -1;
    }

    return 0;
}

int readAddress(char *filename, char *dnsrequest) {
    FILE *file = fopen(filename, "r");

    fscanf(file, "%s", dnsrequest);
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

    struct sockaddr_in server_addr;

    char dnsrequest[MAX_DNS_LEN] = {};
    char dnsresponse[MAX_IP_LEN] = {};

    printf("Client starts...\n");

    address = "127.0.0.1";
    port = atoi("8000");

    if((sockfd = createAndSetSocket(address, port, &server_addr)) < 0) {
        perror("createAndSetSocket");
        exit(1);
    }

    readAddress("domain.txt", dnsrequest);
    printf("Address: %s\n", dnsrequest);

    if(sendToServer(sockfd, &server_addr, dnsrequest, sizeof(dnsrequest)) < 0) {
        perror("dnsrequest");
        exit(1);
    }

    if(receiveFromServer(sockfd, &server_addr, dnsresponse, sizeof(dnsresponse)) < 0) {
        perror("dnsresponse");
        exit(1);
    }

    writeResult("result.txt", dnsresponse);
    printf("Result: %s\n", dnsresponse);

    printf("Client terminates.\n");

    close(sockfd);
    return 0;
}