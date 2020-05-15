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


int starts_with(const char* s, const char* prefix) {
    while (*prefix) {
        if(*s++ != *prefix++) return 0;
    }
    return 1;
}


void init_blocked_list() {
    // Placeholder to load blocked list from a text file
}


int in_blocked_list(const char* hostname) {
    // Hard-coded blocked list for now
    const char* blocked_list[3] = {"example.com", "sample.com", "sing.cse.ust.hk"};

    // Naively scan the entire blocked list for now
    int i;
    for (i = 0; i < 3; ++i) { // hard-coded 3
        if (strcmp(hostname, blocked_list[i]) == 0) return 1;
    }
    return 0;
}


int client_side_draft() { // return type?
    char* request = "GET / HTTP/1.0\r\n\r\n";

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
        }

        recvline[n] = 0;
        printf("%s", recvline); //
    }

    close(sockfd);
    return 0;
}


void https_draft() { // return type?
    // Build a connection with target
    // TODO

    // success
    "HTTP/1.1 200 Connection Established\r\n\r\n"; // version

    // failed

    // may reuse the same socket to avoid reestablishing connection
}


int main() {
    //client_side_draft();
    //return 0;

    init_blocked_list();

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

        int i = 0, n; //
        int j; //
        char recvline[MAX_LINE]; //
        char one_c, four_c[4 + 1]; //
        while (i < sizeof(recvline)) {
            if ((n = read(connfd, &one_c, sizeof(one_c)) <= 0)) { // character by character?
                break;
            }

            recvline[i++] = one_c;

            // this 4 and that 4 are hard-coded
            for (j = 0; j < 4; ++j) four_c[j] = recvline[i - 4 + j];

            // hard-coded
            if (strcmp(four_c, "\r\n\r\n") == 0) break;
        }

        recvline[i] = '\0';

        // bound to not overread

        char method[8] = "FAILED";
        char target_URI[1024]; // length and name
        char protocol_version[32]; // length and name

        // Parse the request line
        sscanf(recvline, "%s %s %s\r\n", method, target_URI, protocol_version);

        if (strcmp(method, "GET") == 0) {
            ; //
        } else if (strcmp(method, "CONNECT") == 0) {
            ; //
        } else {
            // Not supported? HEAD?
            snprintf(buffer, sizeof(buffer), "HTTP/1.1 501 Not Implemented\r\n\r\n"); // version?
            if (write(connfd, buffer, strlen(buffer)) < 0) {
                ; //
            }
            close(connfd);
            continue;
        }

        char hostname[128];
        // TODO
        // 1. target_URI
        // 2. Host header

        if (in_blocked_list(hostname)) { // here?
            snprintf(buffer, sizeof(buffer), "HTTP/1.1 404 Not Found\r\n\r\n"); // version?
            if (write(connfd, buffer, strlen(buffer)) < 0) {
                ; //
            }
            close(connfd);
            continue;
        }

        // Parse the header line(s)
        char* next = recvline; //
        char header[128] = "dummy", field[1024] = "dummy"; //
        while (1) {
            next = strstr(next, "\r\n"); // NULL?
            next += 2;
            if (starts_with(next, "\r\n")) break;
            sscanf(next, "%[^:]: %s\r\n", header, field); // buffer overrun?

            if (strcmp(header, "Proxy-Connection") == 0) {
                ; //
            } else {
                ; //
            }
        }

        // Parse the message body
        // TODO
        // 1. Content-Length if provided
        // 2. ???

        // Construct a request on behalf of the client
        // TODO
        // 1. from absolute to relative
        // 2. from Proxy-Connection to Connection

        // Forward the response
        // TODO

        // -1 for the second parameter?
        snprintf(buffer, sizeof(buffer), "Hello client!\r\n");

        if (write(connfd, buffer, strlen(buffer)) < 0) {
            ; //
        }

        close(connfd);
    }

    return 0;
}


void* f(void* args) {
    ; //
}