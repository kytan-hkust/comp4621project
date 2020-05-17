#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#define LISTENQ  1024
#define MAX_LINE 8192
#define PORT     12345


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


int parse_absolute_url(const char* absolute_url, char* hostname, char* pathname) {
    if (starts_with(absolute_url, "http:")) {
        sscanf(absolute_url, "http://%[^/]%s", hostname, pathname);
    } else if (starts_with(absolute_url, "https:")) {
        sscanf(absolute_url, "https://%[^/]%s", hostname, pathname);
    } else {
        sscanf(absolute_url, "%[^/]%s", hostname, pathname);
    }

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


/* Naive Access Control */
int is_blocked(const char* hostname) {
    const char* blocked_list[2] = {"sing.cse.ust.hk", "google.com"};

    int i;
    for (i = 0; i < 2; ++i) {
        if (strcmp(hostname, blocked_list[i]) == 0) return 1;
    }

    return 0;
}


int is_cached(const char* absolute_url) {
    return 0;
}


ssize_t wrapped_read(int fd, char* buf, size_t count) {
    size_t progress = 0;
    ssize_t n;

    while (progress < count) {
        n = read(fd, buf + progress, count - progress);
        if (n < 0) {
            printf("[ERROR]\tSocket read failed\n");
            return -1;
        } else if (n == 0) break;
        progress += n;
    }
    return n;
}


ssize_t wrapped_write(int fd, char* buf, size_t count) {
    char* t = buf;
    size_t left = count;
    ssize_t n;

    while (left > 0) {
        n = write(fd, t, left);
        t += n;
        left -= n;
    }
    return count;
}


void reply_not_found(int in_fd, const char* version) {
    char buffer[MAX_LINE];
    snprintf(buffer, sizeof(buffer), "%s 404 Not Found\r\n\r\n", version);
    wrapped_write(in_fd, buffer, strlen(buffer));
}


void reply_not_implemented(int in_fd, const char* version) {
    char buffer[MAX_LINE];
    snprintf(buffer, sizeof(buffer), "%s 501 Not Implemented\r\n\r\n", version);
    wrapped_write(in_fd, buffer, strlen(buffer));
}


int obtain_out_fd(const char* hostname) {
    int out_fd;
    struct sockaddr_in servaddr;

    struct timeval t;
    t.tv_sec = 5;
    t.tv_usec = 0;

    out_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(out_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &t, sizeof(t));

    if (out_fd < 0) {
        printf("[ERROR]\tSocket init failed\n");
        return out_fd;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);

    char ip_address[48];
    if (!to_ip_addr(hostname, ip_address)) {
        printf("[ERROR]\tIP address lookup failed\n");
        return -1;
    }
    servaddr.sin_addr.s_addr = inet_addr(ip_address);

    if (connect(out_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("[ERROR]\tSocket connect failed\n");
        return -1;
    }

    return out_fd;
}


void handle_request(int in_fd) {
    char request[MAX_LINE];
    memset(&request, 0, sizeof(request));
    wrapped_read(in_fd, request, MAX_LINE - 1); // -1...?

    char method[8], target[2048], version[16];
    sscanf(request, "%s %s %s\r\n", method, target, version);

    char hostname[2048], pathname[2048];
    parse_absolute_url(target, hostname, pathname);

    if (strcmp(method, "GET") == 0) {
        if (is_blocked(hostname)) reply_not_found(in_fd, version);
        else {
            int out_fd = obtain_out_fd(hostname);
            if (out_fd < 0) {
                //
            }

            char buffer[MAX_LINE];
            snprintf(buffer, sizeof(buffer), "%s %s %s\r\n", method, pathname, version);
            wrapped_write(out_fd, buffer, strlen(buffer));

            char header[1024], field[1024];
            char* next = request;
            while (1) {
                next = strstr(next, "\r\n");
                next += 2;
                if (starts_with(next, "\r\n")) break;
                sscanf(next, "%[^:]: %[^\n]\r\n", header, field);

                if (strcmp(header, "Proxy-Connection") == 0) strcpy(header, "Connection");
                snprintf(buffer, sizeof(buffer), "%s: %s\r\n", header, field);
                wrapped_write(out_fd, buffer, strlen(buffer));
            }
            snprintf(buffer, sizeof(buffer), "\r\n");
            wrapped_write(out_fd, buffer, strlen(buffer));

            int n;
            while (1) {
                n = read(out_fd, buffer, MAX_LINE - 1); // -1...?
                if (n <= 0) break;
                buffer[n] = 0;
                wrapped_write(in_fd, buffer, n); // !
            }

            close(out_fd);
        }
    }
    else if (strcmp(method, "CONNECT") == 0) {
        if (is_blocked(hostname)) reply_not_found(in_fd, version);
        else {
            //
        }
    }
    else reply_not_implemented(in_fd, version);

    close(in_fd);
}


int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr;

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
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) {
            printf("[ERROR]\tSocket accept failed\n");
            return 0;
        }

        struct timeval t;
        t.tv_sec = 5;
        t.tv_usec = 0;

        setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &t, sizeof(t)); //

        handle_request(connfd);
    }

    return 0;
}