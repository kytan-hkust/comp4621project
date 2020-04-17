#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define LISTENQ 42 // or ...?
#define MAGIC 1024 // or ...?
#define PORT 999 // or ...?

int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buffer[MAGIC];
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

        snprintf(buffer, sizeof(buffer), "Hello client!\r\n"); // \r?

        if (write(connfd, buffer, strlen(buffer)) < 0) {
            ; //
        }
    }

    return 0;
}