#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#define LISTENQ     1024  // TODO
#define MAX_LINE    1024  // TODO
#define MAX_THREADS 8
#define PORT        12345


void read_request(int connfd, char* recvline, int L) {
    int i = 0, j, n;
    char c, c4[5];
    while (i < L) {
        if ((n = read(connfd, &c, sizeof(c)) <= 0)) break;
        recvline[i++] = c;

        for (j = 0; j < 4; ++j) c4[j] = recvline[i - 4 + j];
        if (strcmp(c4, "\r\n\r\n") == 0) break;
    }
    recvline[i] = '\0';

    printf("recvline is %s\n", recvline);
}


void* request_handler(void* args) {
    int connfd = (int) args;

    char request_received[MAX_LINE];
    read_request(connfd, request_received, sizeof(request_received));

    close(connfd);
}


int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buffer[MAX_LINE];

    int num_threads = 0;
    pthread_t threads[MAX_THREADS];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[ERROR]\tSocket init failed\n");
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("[ERROR]\tSocket bind failed\n");
        return 0;
    }

    if (listen(listenfd, LISTENQ) < 0) {
        printf("[ERROR]\tSocket listen failed\n");
        return 0;
    }

    while (1) {
        // TODO Type of the 3rd parameter?
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) {
            printf("[ERROR]\tSocket accept failed\n");
            return 0;
        }

        printf("[INFO]\tIncoming request\n");

        if (pthread_create(&threads[num_threads], NULL, request_handler, (void *) connfd) != 0) {
            printf("[ERROR]\tThread %d creation failed\n", num_threads);
            return 0;
        }

        if (++num_threads >= MAX_THREADS) break;
    }

    printf("[LOG] No more threads. Waiting for threads to finish and exit\n");
    int i;
    for (i = 0; i < MAX_THREADS; ++i) pthread_join(threads[i], NULL);

    return 0;
}