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

int main() {
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