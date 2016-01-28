#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <error.h>
#include <bits/signum.h>
#include <signal.h>


#define MAxSIZE 4096
#define MAX_CONNECT 10000


using namespace std;

void* pthread_handle_message(void* para);
int pthread_init_pool(int thread_num, int epfd);

int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0)|O_NONBLOCK) == -1)
    {
        return -1;
    }
    return 0;
}

int conSocket, epollfd;

int main() {


    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

   // int epollfd;
    struct sockaddr_in serAddr;
    struct epoll_event ev;
    //struct epoll_event events[MAX_CONNECT];

    if((conSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("create socket error");
        exit(0);
    }

    int opt = SO_REUSEADDR;
    setsockopt(conSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /*no block*/
    setnonblocking(conSocket);

    memset(&serAddr, 0, sizeof(serAddr));
    serAddr.sin_addr.s_addr = inet_addr("192.168.207.128");
    serAddr.sin_port = htons(8000);
    serAddr.sin_family = AF_INET;

    if(bind(conSocket, (struct sockaddr*)&serAddr, sizeof(serAddr)))
    {
        perror("bind socker error");
        exit(0);
    }

    if(listen(conSocket,MAX_CONNECT))
    {
        perror("listen socker error");
        exit(0);
    }

    epollfd = epoll_create(MAX_CONNECT);

    int len = sizeof(struct sockaddr_in);

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = conSocket;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, conSocket, &ev) < 0)
    {
        perror("epoll_ctr");
        exit(0);
    }

    pthread_init_pool(10, epollfd);


    pthread_exit(NULL);
    return 0;

//    int curfds = 1;
//    int nfds, newfd;

//    while(1)
//    {
//        if((nfds = epoll_wait(epollfd, events, curfds, -1)) < 0)
//        {
//            printf("epoll_wait curfds  = %d\n", curfds);
//            continue;
//        }
//
//        for (int i = 0; i < nfds; ++i) {
//            if(events[i].data.fd == conSocket) {
//                newfd = accept(conSocket, (struct sockaddr *) NULL, NULL);
//                if (newfd < 0) {
//                    perror("accept ");
//                }
//
//                setnonblocking(newfd);
//                ev.events = EPOLLIN | EPOLLET;
//                ev.data.fd = newfd;
//                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, newfd, &ev) < 0) {
//                    printf("epoll_ctl error! newfd = %d\n", newfd);
//                    exit(1);
//                }
//
//
//                curfds++;
//
//                printf("curfds = %d \n", curfds);
//
//            }else
//            {
//                pthread_attr_t attr;
//                pthread_t  threadId;
//
//                pthread_attr_init(&attr);
//                pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
//                pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
//
//                if(pthread_create(&threadId, &attr, pthread_handle_message, (void *)&(events[i].data.fd)))
//                {
//                    perror("pthread_create error!");
//                    exit(-1);
//                }
//
//            }
//        }
//
//
//    }




}


int pthread_init_pool(int thread_num, const int epfd)
{
    pthread_t threadId;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    void *statue;
    int rc, fd;

    long threads[thread_num];

    for (int i = 0; i < thread_num; ++i) {

        fd = epfd;

        if(rc = pthread_create(&threadId, &attr, pthread_handle_message, (void *)&fd))
        {
            printf("ERROR; return code from pthread_create()is %d\n", rc);
            exit(-1);
        }
        threads[i] = threadId;

        printf("create thread id = %ld\n", threadId);

//        if(rc = pthread_join(threadId, &statue) )
//        {
//            printf("ERROR; return code from pthread_join() is %d\n", rc);
//            exit(-1);
//        }
    }

    for (int j = 0; j < thread_num; ++j) {

        if(rc = pthread_join(threads[j], &statue) )
        {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }

    return  0;
}

void*  pthread_handle_message(void *param)
{
    int epfd = *(int*) param;
    //int epfd = epollfd;
    printf("epollfd = %d\n", epfd);
    char buff[2048];
    int newfd;
    int nrecv, nfd;

    epoll_event events[MAX_CONNECT];
    while(1) {
        if ((nfd = epoll_wait(epfd, events, MAX_CONNECT, -1)) < 0) {
            printf("epoll_wait fail nfd = %d, errno = %d\n", nfd, errno);
            break;

        }

        printf("nfd = %d\n", nfd);

        for (int i = 0; i < nfd; ++i) {
            if (events[i].data.fd == conSocket) {
                while (1) {

                    newfd = accept(conSocket, (struct sockaddr *) NULL, NULL);
                    if (newfd > 0) {
                        setnonblocking(newfd);
                        epoll_event event;
                        event.data.fd = newfd;
                        event.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET;
                        epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &event);
                    } else {
                        if (errno == EAGAIN)
                            break;
                    }
                }
            }
            else {
                if (events[i].events & EPOLLIN) {
                    int contiFlag = true;
                    std::string msg;
                    while (contiFlag) {
                        memset(buff, 0, sizeof(buff));
                        nrecv = recv(events[i].data.fd, buff, sizeof(buff), 0);
                        if (nrecv < 0) {
                            if (errno == EAGAIN) {
                                continue;
                            }
                            else {
                                perror("recv fail1");
                                epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                                close(events[i].data.fd);
                                break;
                            }
                        }
                        else if (nrecv == 0) {
                            printf("client close\n");
                            epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                            close(events[i].data.fd);
                            break;
                        }
                        else {
                            msg.append(buff, strlen(buff));
                        }

                        if (nrecv < sizeof(buff)) {
                            contiFlag = false;
                        }
                    }

                    if(!msg.empty())
                    {
                        printf("recv form client fd(%d):%s\n", events[i].data.fd, msg.c_str());
                        send(events[i].data.fd, "good", 4, 0);
                    }
                }

                else if (events[i].events & EPOLLOUT) {
                    int thread_id = pthread_self();
                    printf("send to client from thread_id = %ld\n", thread_id);
                    send(events[i].data.fd, "good", 4, 0);
                }

                else {
                    perror("recv fail 2");
                    break;
                }

            }
        }
    }

    pthread_exit(NULL);
};

