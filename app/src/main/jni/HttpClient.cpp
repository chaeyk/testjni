//
// Created by chaeyk on 2016-05-19.
//

#include "HttpClient.h"

//#include <stdlib.h>
#include <stdio.h>
//#include <string.h>
//#include <fcntl.h>
//#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <android/log.h>

int HttpGet(const char *server, int port, const char *url) {
    __android_log_print(ANDROID_LOG_INFO, "-----from--jni-------", "Enter HttpGet function!");

    /*
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    __android_log_print(ANDROID_LOG_INFO, "-----from--jni-------", "%d", sock);
    */

    int ret;

    char port_str[10];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    ret = getaddrinfo(server, port_str, &hints, &result);
    if (ret != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "-----from--jni-------", "resolv failed!");
        return -1;
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */
    int sock;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        __android_log_print(ANDROID_LOG_DEBUG, "testjni", "family-%d sock-%d protocol-%d ip-%s",
                            (int) rp->ai_family, (int) rp->ai_socktype, (int) rp->ai_protocol, inet_ntoa(((struct sockaddr_in*) rp->ai_addr)->sin_addr));

        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;
        ret = connect(sock, rp->ai_addr, rp->ai_addrlen);
        if (ret != 0) {
            __android_log_print(ANDROID_LOG_ERROR, "-----from--jni-------", "connect failed: %d %s", ret, strerror(errno));
            close(sock);
            continue;
        }

        break;
    }
    freeaddrinfo(result);
    if (rp == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, "-----from--jni-------", "all connect failed!");
        return -1;
    }

    char buf[2048];
    sprintf(buf,
            "GET %s HTTP/1.1\r\n"
            "Accept: */*\r\n"
            "User-Agent: bbm-crash-handler\r\n"
            "Host: %s\r\n"
            "Connection: Close\r\n\r\n",
            url,
            server);
    send(sock, buf, strlen(buf), 0);

    __android_log_print(ANDROID_LOG_INFO, "-----from--jni-------", "Enter HttpGet function's while!");
    while ((ret = recv(sock, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[ret] = 0;
        __android_log_print(ANDROID_LOG_INFO, "-----from--jni-------", "%s", buf);
    }
    shutdown(sock, SHUT_RDWR);
    close(sock);

    return 0;
}
