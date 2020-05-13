#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#define LISTENQ 42 // or ...?
#define MAX_LINE 1024 // or ...?
#define MAX_THREADS 42 // or ...?
#define PORT 999 // or ...?


int client_side_draft() {
    char *request = "GET / HTTP/1.0\r\n\r\n";

    int sockfd, n;
    char writeline[MAX_LINE], recvline[MAX_LINE]; // length?
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ; //
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80); // 80 for http, 443 for https

    // switch to inet_pton?
    servaddr.sin_addr.s_addr = inet_addr("172.217.25.14"); // hard-coded google.com

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        ; //
    }

    snprintf(writeline, sizeof(writeline), request); // sizeof(.) - 1?

    write(sockfd, writeline, strlen(writeline));

    while (1) {
        if ((n = read(sockfd, recvline, sizeof(recvline) - 1)) <= 0) {
            break;
        } else {
            recvline[n] = 0;
            printf("%s", recvline); //
        }
    }

    close(sockfd);
    return 0;
}


int main() {
    client_side_draft();
    return 0;

    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buffer[MAX_LINE];

    int num_threads = 0;
    pthread_t threads[MAX_THREADS];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ; //
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // AF_INET6?
    servaddr.sin_addr.s_addr = INADDR_ANY; // htonl?
    servaddr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        ; //
    }

    if (listen(listenfd, LISTENQ) < 0) {
        ; //
    }

    while (1) {
        // type of the 3rd parameter?
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) {
            ; //
        }

        printf("Incoming connection!\n");

        // -1 for the second parameter?
        snprintf(buffer, sizeof(buffer), "Hello client!\r\n"); // \r?

        if (write(connfd, buffer, strlen(buffer)) < 0) {
            ; //
        }

        close(connfd);
    }

    return 0;
}


void* f(void *args) {
    ; //
}