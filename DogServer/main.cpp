#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 8000
#define MAXLINE 4096
#define BACKLOG 5

int fd_A[BACKLOG];
int conn_amount;

using namespace std;

int main(int argc, char** argv)
{
    int socket_fd, connect_fd;
    char buff[MAXLINE];
    int n;

    struct sockaddr_in serverAddr;


    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("create socket error:%s(error:%d)\n", strerror(errno), errno);
        exit(0);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.207.128");

    if(bind(socket_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printf("bind socket error:%s(error:%d)\n", strerror(errno), errno);
        exit(0);
    }

    if(listen(socket_fd, 100) == -1)
    {
        printf("listen socket error:%s(error:%d)\n", strerror(errno), errno);
        exit(0);
    }

//    if((connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1)
//    {
//        printf("accept socket error:%s(error:%d)\n", strerror(errno), errno);
//    }

    fd_set fdsr;
    struct timeval tv;

    int maxsock = socket_fd;
    int connectCount = 0;




    while(1)
    {


        FD_ZERO(&fdsr);
        FD_SET(socket_fd, &fdsr);
        for (int k = 0; k < BACKLOG; ++k) {
            if (fd_A[k] != 0)
            {
                FD_SET(fd_A[k], &fdsr);
            }
        }
        tv.tv_sec = 30;
        tv.tv_usec = 0;

        //after exec select() ,the param fdsr, tv will be changed
        //so need init fdsr and tv everytime

        int ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
        if(ret < 0 )
        {
            perror("select error\n");
            break;
        }
        else if(ret == 0)
        {
            printf("timeout\n");
            continue;
        }

        for (int j = 0; j < BACKLOG; ++j) {
            if(FD_ISSET(fd_A[j], &fdsr))
            {
                memset(&buff, 0, sizeof(buff));
                int res = recv(fd_A[j], buff, sizeof(buff), 0);

                if(res <= 0 && errno != EINTR)
                {
                    printf("socketId %d close\n ", fd_A[j]);
                    close(fd_A[j]);
                    FD_CLR(fd_A[j], &fdsr);
                    fd_A[j] = 0;

                    connectCount -- ;
                }
                else
                {
                    buff[res] = '\0';
                    printf("socketId %d recv:%s \n", fd_A[j], buff);

                    if(send(fd_A[j], "welcome to connect me", 21, 0) < 0 )
                    {
                        perror("send error");
                    }
                }
            }
        }

        if(FD_ISSET(socket_fd, &fdsr))
        {
            if (connectCount >= BACKLOG)
            {
                perror("Connect More Than Max !!");
                continue;
            }
            int newCConnect = accept(socket_fd, (struct sockaddr*)NULL, NULL);
            if (newCConnect <= 0)
            {
                perror("accept fail");
                continue;
            }

            for (int i = 0; i < BACKLOG; ++i) {
                if(fd_A[i] == 0)
                {
                    fd_A[i] = newCConnect;
                    FD_SET(fd_A[i], &fdsr);
                    maxsock = newCConnect;

                    printf("new connect socket Id %d\n", newCConnect);
                    break;
                }
            }
        }

    }

    close(socket_fd);


    return 0;
}







