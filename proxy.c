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


int starts_with(const char* s, const char* prefix) {
    while (*prefix) {
        if (*s++ != *prefix++) return 0;
    }
    return 1;
}


int to_ip_addr(const char* hostname, char* ip_address) {
    struct hostent* h;
    if ((h = gethostbyname(hostname)) == NULL) return 0;

    struct in_addr** ip_addresses = (struct in_addr **) h->h_addr_list;
    if (ip_addresses == NULL || ip_addresses[0] == NULL) return 0;

    char* s = inet_ntoa(*ip_addresses[0]);
    strcpy(ip_address, s);
    return 1;
}


int parse_absolute(const char* absolute, char* hostname, char* pathname) {
    if (starts_with(absolute, "http:")) sscanf(absolute, "http://%[^/]%s", hostname, pathname);
    else if (starts_with(absolute, "https:")) sscanf(absolute, "https://%[^/]%s", hostname, pathname);
    else sscanf(absolute, "%[^/]%s", hostname, pathname);

    if (starts_with(hostname, "www.")) {
        int i = 0;
        while (hostname[i + 4] != '\0') {
            hostname[i] = hostname[i + 4];
            ++i;
        }
        hostname[i] = '\0';
    }

    if (strcmp(pathname, "") == 0) {
        pathname[0] = '/';
        pathname[1] = '\0';
    }

    int port = 0;
    char* t = strstr(hostname, ":");
    if (t) {
        *t = '\0';
        ++t;
        while (*t != '\0') {
            port *= 10;
            port += *t - '0';
            ++t;
        }
    }
    return port;
}


int is_blocked(const char* hostname) {
    return 0;
}


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

    // Parse the request line
    char method[8], target[2048], version[16];
    sscanf(recvline, "%s %s %s\r\n", method, target, version);

    if (strcmp(method, "GET") == 0) ;           // TODO
    else if (strcmp(method, "CONNECT") == 0) ;  // TODO
    else ;                                      // TODO

    char hostname[2048], pathname[2048];
    parse_absolute(target, hostname, pathname);

    if (is_blocked(hostname)) ;                 // TODO

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