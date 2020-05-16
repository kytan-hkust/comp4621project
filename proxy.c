#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#define PORT    12345
#define LISTENQ 1024


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
    const char* blocked_list[3] = {"sample.com", "example.com", "google.com"};

    int i;
    for (i = 0; i < 3; ++i) {
        if (strcmp(hostname, blocked_list[i]) == 0) return 1;
    }

    return 0;
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
        break;
    }

    return 0;
}