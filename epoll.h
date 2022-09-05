#ifndef _EPOLL_H_
#define _EPOLL_H_
#include"address.h"
#include<sys/epoll.h>
namespace htb
{
class EpollFunc
{
    public:
        static void addWriteFdLT( int epollfd, int fd);
        static void addRecvFdLT( int epollfd, int fd);
        static void addWriteFdET( int epollfd, int fd);
        static void addRecvFdET( int epollfd, int fd);
        static void removeFd(int epollfd, int fd);
        static int setNonBlock( int m_sock );
};
}
#endif