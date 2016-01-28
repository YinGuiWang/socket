#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SRPORT  8000
#define MAXSIZE 4096

using namespace std;



int main() {

    int socket_fd;
    struct sockaddr_in serverAddr;
    char buff[MAXSIZE], sendinfo[MAXSIZE];
    int rec_len;

    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("create socket error:%s(errno:%d)\n", strerror(errno), errno);
        exit(0);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SRPORT);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.207.128");
    if(connect(socket_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printf("connect error:%s(errno:%d)\n", strerror(errno), errno);
        exit(0);
    }

    printf("send msg to Server:\n");
    fgets(sendinfo, MAXSIZE, stdin);
    if(send(socket_fd, sendinfo, strlen(sendinfo), 0) < 0)
    {
        printf("send error:%s(errno:%d)\n", strerror(errno), errno);
        exit(0);
    }

    if((rec_len = recv(socket_fd, buff,  MAXSIZE, 0)) == -1)
    {
        perror("recv error");
        exit(0);
    }
    buff[rec_len] = '\0';
    printf("rec:%s\n", buff);


    int socket_fd2;
    if((socket_fd2 = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {

        printf("create socket2 error:%s(errno:%d)", strerror(errno), errno);
        exit(0);
    }

    if(connect(socket_fd2, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printf("connect2  error:%s(errno:%d)", strerror(errno), errno);
        exit(0);
    }

    printf("send2 msg to Server:\n");
    memset(&sendinfo, 0, sizeof(sendinfo));
    fgets(sendinfo, MAXSIZE, stdin);
    if(send(socket_fd2, sendinfo, strlen(sendinfo), 0) < 0)
    {
        printf("send error:%s(errno:%d)\n", strerror(errno), errno);
        exit(0);
    }

    if((rec_len = recv(socket_fd2, buff,  MAXSIZE, 0)) == -1)
    {
        perror("recv error");
        exit(0);
    }
    buff[rec_len] = '\0';
    printf("rec:%s\n", buff);


    printf("send msg to Server:\n");
    memset(&sendinfo, 0, sizeof(sendinfo));
    fgets(sendinfo, MAXSIZE, stdin);
    if(send(socket_fd, sendinfo, strlen(sendinfo), 0) < 0)
    {
        printf("send error:%s(errno:%d)\n", strerror(errno), errno);
        exit(0);
    }

    if((rec_len = recv(socket_fd, buff,  MAXSIZE, 0)) == -1)
    {
        perror("recv error");
        exit(0);
    }
    buff[rec_len] = '\0';
    printf("rec:%s", buff);

    close(socket_fd2);
    close(socket_fd);
    //exit(0);
   // sleep(1000);
}