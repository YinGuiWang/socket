#include <iostream>

using namespace std;

#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define  CLIENT_NUM 5000


void setnonblocking(int socketfd)
{
    int op = fcntl(socketfd, F_GETFL);
    op =  op | O_NONBLOCK;
    fcntl(socketfd, op);
}


void addfd(int socketfd, int epollfd)
{
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR;
    ev.data.fd = socketfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev);

    setnonblocking(socketfd);
}

void closefd(int socketfd, int epollfd)
{
    close(socketfd);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd, NULL);
}

int main() {

    int clientfd;

    char buff[2048];

    int epollfd = epoll_create(CLIENT_NUM);

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8000);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.207.128");

    for (int i = 0; i < CLIENT_NUM; ++i) {

        if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket error");
            exit(0);
        }

        if (connect(clientfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
            perror("connect error");
            exit(0);

        }

        printf("connect success %d \n", i);

        addfd(clientfd, epollfd);
    }

    epoll_event events[CLIENT_NUM];
    int nfd, nrecv, connectCount;
    int newfd;
    while (1) {
        if ((nfd = epoll_wait(epollfd, events, sizeof(events), -1)) < 0) {
            perror("epoll_wait empty!!");
            continue;
        }

        for (int i = 0; i < nfd; ++i) {

            if (events[i].events & EPOLLIN) {
                memset(buff, 0, sizeof(0));
                if ((nrecv = recv(events[i].data.fd, buff, sizeof(buff), 0)) < 0) {
                    perror("recv data fail");
                    continue;
                }

                buff[nrecv] = '\0';
                //epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                printf("recv from server : %s \n", buff);

                //close(events[i].data.fd);
            }
            else if (events[i].events & EPOLLOUT) {

                if(send(events[i].data.fd, "I am a good body", 16, 0) < 0)
                {
                    perror("send to server success!");
                }
            }

            else if(events[i].events & EPOLLERR)
            {
                closefd(events[i].data.fd, epollfd);
            }
        }
    }


    cout << "Hello, World!" << endl;
    return 0;
}