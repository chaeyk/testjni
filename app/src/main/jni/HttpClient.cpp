//
// Created by chaeyk on 2016-05-19.
//

#include "HttpClient.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <android/log.h>

int GetHttpResponseHead(int sock,char *buf,int size)
{
    int i;
    char *code,*status;
    memset(buf,0,size);
    for(i=0; i<size-1; i++){
        if(recv(sock,buf+i,1,0)!=1){
            perror("recv error");
            return -1;
        }
        if(strstr(buf,"\r\n\r\n"))
            break;
    }
    if(i >= size-1){
        puts("Http response head too long.");
        return -2;
    }
    code=strstr(buf," 200 ");
    status=strstr(buf,"\r\n");
    if(!code || code>status){
        *status=0;
        printf("Bad http response:\"%s\"\n",buf);
        return -3;
    }
    return i;
}

int HttpGet(const char *server,const char *url)
{
    __android_log_print(ANDROID_LOG_INFO, "-----from--jni-------", "Enter HttpGet function!");
    int sock=socket(PF_INET,SOCK_STREAM,0);
    __android_log_print(ANDROID_LOG_INFO, "-----from--jni-------", "%d",sock);
    struct sockaddr_in peerAddr;
    char buf[2048];
    int ret;

    peerAddr.sin_family=AF_INET;
    peerAddr.sin_port=htons(3009);
    peerAddr.sin_addr.s_addr=inet_addr(server);
    ret=connect(sock,(struct sockaddr *)&peerAddr,sizeof(peerAddr));
    if(ret != 0){
        perror("connect failed");
        close(sock);
        return -1;
    }
    sprintf(buf,
            "GET %s HTTP/1.1\r\n"
                    "Accept: */*\r\n"
                    "User-Agent: bbm-crash-handler\r\n"
                    "Host: %s\r\n"
                    "Connection: Close\r\n\r\n",
            url,server);
    send(sock,buf,strlen(buf),0);
    if(GetHttpResponseHead(sock,buf,sizeof(buf))<1){
        close(sock);
        return -1;
    }
    __android_log_print(ANDROID_LOG_INFO, "-----from--jni-------", "Enter HttpGet function's while!");
    while((ret=recv(sock,buf,sizeof(buf),0)) > 0)
    {
        __android_log_print(ANDROID_LOG_INFO, "-----from--jni-------", "hello, this is my number %s log message!", buf);
    }
    shutdown(sock,SHUT_RDWR);
    close(sock);

    return 0;
}
