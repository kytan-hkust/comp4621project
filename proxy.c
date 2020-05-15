#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#define LISTENQ 42 // or ...?
#define MAX_LINE 8192 // or ...?
#define MAX_THREADS 42 // or ...?
#define PORT 12345


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
    const int BLOCKED_LIST_LEN = 4; // hard-coded;
    // Hard-coded blocked list for now
    // const char* blocked_list[3] = {"example.com", "sample.com", "sing.cse.ust.hk"}; // 3...
    const char* blocked_list[4] = {"example.com", "www.example.com", "http://www.example.com", "http://example.com"};

    // Naively scan the entire blocked list for now
    int i;
    for (i = 0; i < BLOCKED_LIST_LEN; ++i) {
        if (strcmp(hostname, blocked_list[i]) == 0) return 1;
    }
    return 0;
}


int to_relative(const char* absolute, char* r1, char* r2) {
    // TODO: from absolute to relative
    // %[^:/]
    if (starts_with(absolute, "http:")) {
        sscanf(absolute, "http://%[^/]%s", r1, r2);
    }
    else if (starts_with(absolute, "https:")) {
        sscanf(absolute, "https://%[^/]%s", r1, r2);
    }
    else sscanf(absolute, "%[^/]%s", r1, r2);

    if (starts_with(r1, "www.")) {
        ; // TODO trim
    }

    if (strcmp(r2, "") == 0) {
        r2[0] = '/';
        r2[1] = '\0';
    } else if (starts_with(r2, ":")) {
        ; // TODO port
    }

    return 1;
}


int to_ip_addr(const char* hostname, char* ip_address) {
    struct hostent* h;
    if ((h = gethostbyname(hostname)) == NULL) {
        ; //
        return 0;
    }

    struct in_addr** ip_addresses = (struct in_addr **) h->h_addr_list;
    if (ip_addresses == NULL || ip_addresses[0] == NULL) {
        ; //
        return 0;
    }

    char* s = inet_ntoa(*ip_addresses[0]);
    strcpy(ip_address, s); // buffer overflow..., IPv4 only?
    return 1;
}


int client_side_draft(const char* request, const char* hostname) {
    ; // TODO...
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
    init_blocked_list();

    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buffer[MAX_LINE];

    int num_threads = 0;
    pthread_t threads[MAX_THREADS];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[ERROR]\tinit socket\n"); //
        return 0; //
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // AF_INET6?
    servaddr.sin_addr.s_addr = INADDR_ANY; // htonl?
    servaddr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("[ERROR]\tbind\n"); //
        return 0; //
    }

    if (listen(listenfd, LISTENQ) < 0) {
        printf("[ERROR]\tlisten\n"); //
        return 0; //
    }

    while (1) {
        // type of the 3rd parameter?
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) {
            printf("[ERROR]\taccept\n"); //
            return 0; //
        }

        printf("[INFO]\tIncoming request\n");

        int content_length = 0;
        int i = 0, n; //
        int j; //
        char recvline[MAX_LINE]; //
        char one_c, four_c[4 + 1]; //
        while (i < sizeof(recvline)) {
            if ((n = read(connfd, &one_c, sizeof(one_c)) <= 0)) { // character by character?
                break;
            }

            content_length += n;

            recvline[i++] = one_c;

            // this 4 and that 4 are hard-coded
            for (j = 0; j < 4; ++j) four_c[j] = recvline[i - 4 + j];

            // hard-coded
            if (strcmp(four_c, "\r\n\r\n") == 0) break;
        }

        recvline[i] = '\0';
        printf("[INFO]\tReceived request from client\n");

        // bound to not overread

        char method[8];
        char target_URI[1024]; // length and name, will be used for caching (absolute)
        char protocol_version[32]; // length and name

        // Parse the request line
        char request[MAX_LINE];
        char* tmp = request;

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

        char r1[128], r2[128]; //
        to_relative(target_URI, r1, r2);

        // Construct a request on behalf of the client
        strcpy(tmp, method); //
        tmp += strlen(method);
        strcpy(tmp, " "); //
        ++tmp;;
        strcpy(tmp, r2);
        tmp += strlen(r2);
        strcpy(tmp, " ");
        ++tmp;;
        strcpy(tmp, protocol_version);
        tmp += strlen(protocol_version);
        strcpy(tmp, "\r\n");
        tmp += 2;

        // Parse the header line(s)
        char* next = recvline; //
        char header[128] = "", field[1024] = ""; //
        while (1) {
            next = strstr(next, "\r\n"); // NULL?
            next += 2;
            if (starts_with(next, "\r\n")) break;
            sscanf(next, "%[^:]: %s\r\n", header, field); // buffer overrun?

            if (strcmp(header, "Host") == 0) strcpy(hostname, field); // port might follow
            else if (strcmp(header, "Proxy-Connection") == 0) strcpy(header, "Connection");
            else if (strcmp(header, "Content-Length") == 0) ; // TODO

            strcpy(tmp, header);
            tmp += strlen(header);
            strcpy(tmp, ": ");
            tmp += 2;
            strcpy(tmp, field);
            tmp += strlen(field);
            strcpy(tmp, "\r\n");
            tmp += 2;
        }

        strcpy(tmp, "\r\n");
        tmp += 2;

        if (in_blocked_list(hostname)) { // TODO: move to the appropriate location
            printf("[LOG] Website blocked\n");
            // TODO: fix this bug
            snprintf(buffer, sizeof(buffer), "HTTP/1.1 404 Not Found\r\n\r\n"); // version?
            if (write(connfd, buffer, strlen(buffer)) < 0) {
                printf("[ERROR]\twrite\n"); //
                return 0; //
            }
            close(connfd);
            continue;
        }

        // TODO
        // 1. Content-Length if provided
        // 2. ???

        // TODO
        // Move the message body to the remaining part of request

        int written = tmp - request;
        while (written < content_length) {
            *tmp = request[written];
            ++written;
            ++tmp;
        }
        // '\0' at the end
        *tmp = '\0';

        printf("[LOG] Prepare to request on behalf of client\n");

        // Forward the response
        // TODO
        char some_buffer[MAX_LINE] = "HTTP/1.1 200 OK\r\n\r\n<html>Hi</html>"; // just for debug
        client_side_draft(request, hostname); // TODO: fix hostname

        printf("writing back to client step 2\n");

        // -1 for the second parameter?
        snprintf(buffer, sizeof(buffer), some_buffer);

        if (write(connfd, buffer, strlen(buffer)) < 0) {
            printf("[ERROR]\twrite\n"); //
            return 0; //
        }

        printf("[LOG]\tForwarded the response to client\n");

        close(connfd);
    }

    return 0;
}


void* f(void* args) {
    ; //
}