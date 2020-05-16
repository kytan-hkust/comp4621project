#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

// #include <fcntl.h> // please...

#define LISTENQ     64  // TODO
#define MAX_LINE    1024  // TODO
#define MAX_THREADS 16    // TODO
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


int read_request(int connfd, char* recvline, int L) {
    int i = 0, j, n;
    char c, c4[5];
    while (i < L) {
        n = read(connfd, &c, sizeof(c));
        if (n < 0) return 1;
        else if (n == 0) break;
        recvline[i++] = c;

        for (j = 0; j < 4; ++j) c4[j] = recvline[i - 4 + j];
        if (strcmp(c4, "\r\n\r\n") == 0) break;
    }
    recvline[i] = '\0';
    return 0;
}


int send_request(const char* request, const char* hostname, int forward_fd) {
    struct timeval t;
    t.tv_sec = 3; // hacky...
    t.tv_usec = 0;

    printf("[DEBUG]\thostname = %s, request = [%s]\n", hostname, request);
    int sockfd, n;
    char writeline[MAX_LINE] = {0};
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_NONBLOCK...

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &t, sizeof(t));

    if (sockfd < 0) {
        printf("[ERROR]\tSocket init failed\n");
        return 1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80); // TODO 80 for http, 443 for https

    char ip_address[48];
    if (to_ip_addr(hostname, ip_address)) {
        printf("[LOG]\tIP address lookup succeeded: %s %s\n", ip_address, hostname);
    }
    else {
        printf("[ERROR]\tIP address lookup failed\n");
        return 1; // TODO
    }

    servaddr.sin_addr.s_addr = inet_addr(ip_address); // TODO switch to inet_pton?

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("[ERROR]\tSocket connect failed\n");
        return 1;
    }

    // fcntl(sockfd, F_SETFL, O_NONBLOCK); // please.........

    snprintf(writeline, sizeof(writeline), request); // -1??? no loop?
    int L = strlen(writeline);
    char* _writeline = writeline;

    while (1) {
        n = write(sockfd, _writeline, L);
        if (n < 0) {
            printf("[ERROR] Socket write failed\n");
            exit(0);
        } else if (n == 0) {
            printf("[LOG] Socket write done\n");
            break;
        } else {
            L -= n;
            _writeline += n;
        }
    }

    printf("[LOG]\tReceiving the response from target\n");

    // Version 1
    char recvline[MAX_LINE] = {0}; // TODO better name, FOR Forward NOW
    int debug = 0;
    while (1) {
        ++debug;
        n = read(sockfd, recvline, sizeof(recvline)); // because this blocks...???
        if (n <= 0) {
            printf("[DEBUG] Breaking out from this endless loop %d\n", debug);
            break;
        }
        else {
            recvline[n] = 0;
            printf("%s", recvline); // removed \n

            //printf("----FREEZE---- n = %d, %d-th loop\n", n, debug);
        }
    }


    // Version 2
    // TODO the endless loop
    // int bytes_read = 0;
    // while (bytes_read < MAX_LINE) {
    //     n = read(sockfd, recvline + bytes_read, MAX_LINE - bytes_read);
    //     if (n <= 0) {
    //         break;
    //     }
    //     bytes_read += n;
    // }
    // TODO
    // recvline[bytes_read] = '\0';

    printf("[LOG]\tReceived the response from target\n");



    // Debugging forwarding response back to client...
    printf("[DEBUG]\tForwarding to %d\n", forward_fd);
    char send_help[MAX_LINE] = {0};
    // - 1?
    snprintf(send_help, sizeof(send_help) - 1, "HTTP/1.1 200 OK\r\n\r\n<html>Debugging response forwarding...</html>");

    int L2 = strlen(send_help);
    char* _send_help = send_help;

    while (1) {
        n = write(forward_fd, _send_help, L2);
        if (n < 0) {
            printf("[ERROR] Socket write failed\n");
            exit(0);
        } else if (n == 0) {
            printf("[LOG] Socket write done\n");
            break;
        } else {
            L2 -= n;
            _send_help += n;
        }
    }

    close(sockfd);
    return 0;
}


//void* request_handler(void* args) {
void request_handler(int args) {
    printf("[LOG]\tHandling incoming request\n");

    int connfd = (int) args;

    char recvline[MAX_LINE] = {0};
    int r = read_request(connfd, recvline, sizeof(recvline));
    if (r) return;

    // Parse the request line
    char method[8], target[2048], version[16];
    sscanf(recvline, "%s %s %s\r\n", method, target, version);

    if (strcmp(method, "GET") == 0) {
        // TODO
    }
    else if (strcmp(method, "CONNECT") == 0) {
        // TODO
        printf("[LOG]\t Dropped HTTPS request\n");
        close(connfd);
        return; /////
        ///// return NULL;
        // TODO
    }
    else ; // TODO

    // Parse the absolute URL
    char hostname[2048], pathname[2048];
    parse_absolute(target, hostname, pathname);

    // Check if it's blocked
    if (is_blocked(hostname)) ;                 // TODO

    char buffer[MAX_LINE] = {0};                // TODO better name
    int n, L = sizeof(buffer);
    char* _buffer = buffer;

    n = snprintf(_buffer, L, "%s %s %s\r\n", method, pathname, version);
    _buffer += n;
    L -= n;

    // Parse the header line(s)
    char header[1024], field[1024];
    char* next = recvline;
    while (1) {
        next = strstr(next, "\r\n");
        next += 2;
        if (starts_with(next, "\r\n")) break;
        sscanf(next, "%[^:]: %[^\n]\r\n", header, field); // !

        if (strcmp(header, "Proxy-Connection") == 0) strcpy(header, "Connection");

        n = snprintf(_buffer, L, "%s: %s\r\n", header, field);
        _buffer += n;
        L -= n;
    }

    // TODO
    // "\r\n" at the end?
    // '\0' at the end?
    n = snprintf(_buffer, L, "\r\n");
    _buffer += n;
    L -= n;
    *_buffer = '\0';

    // TODO
    // Copy and paste the message body

    // TODO

    // printf("[DEBUG]\tBuffer contains [%s]\n", buffer);

    // char response[MAX_LINE] = "HTTP/1.1 200 OK\r\n\r\n<html>Send help...</html>";
    send_request(buffer, hostname, connfd);

    printf("[DEBUG]\tExited send_request\n");

    // TODO Forward the request to client

    close(connfd);
}


int main() {
    //char dumb[MAX_LINE];
    //send_request("GET / HTTP/1.1\r\n\r\n", "sing.cse.ust.hk", dumb, 0);
    // 1.0 for sing.cse.ust.hk is okay... didn't freeze...
    // 1.0 for www.google.com is okay... didn't freeze...     (1.1 freezes)
    // 1.0 for www.sample.com is also okay... didn't freeze   (1.1 freezes)

    //return 0;

    int listenfd, connfd;
    struct sockaddr_in servaddr;

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
        // TODO 3rd parameter?
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, (socklen_t *) NULL)) < 0) {
            printf("[ERROR]\tSocket accept failed\n");
            return 0;
        }

        request_handler(connfd);

        // if (pthread_create(&threads[num_threads], NULL, request_handler, (void *) connfd) != 0) {
        //     printf("[ERROR]\tThread %d creation failed\n", num_threads);
        //     return 0;
        // }

        // if (++num_threads >= MAX_THREADS) break;
    }

    // printf("[LOG] No more threads. Waiting for threads to finish and exit\n");
    // int i;
    // for (i = 0; i < MAX_THREADS; ++i) pthread_join(threads[i], NULL);

    return 0;
}