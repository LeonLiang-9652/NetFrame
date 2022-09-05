#include"epoll.h"
#include<iostream>
namespace htb
{
    void EpollFunc::addWriteFdLT( int epollfd, int fd)
    {
        epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLOUT;
        epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event);
    }
    void EpollFunc::addRecvFdLT( int epollfd, int fd)
    {
        epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN|EPOLLRDHUP;
        epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event);
    }
    void EpollFunc::addWriteFdET( int epollfd, int fd)
    {
        epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLOUT|EPOLLET|EPOLLONESHOT;
        epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event);

    }
    void EpollFunc::addRecvFdET( int epollfd, int fd)
    {
        epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLONESHOT;
        if(epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event)==-1)
            std::cout<<"error"<<std::endl;
        
    }
    void EpollFunc::removeFd(int epollfd, int fd)
    {
        //epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0);
        close(fd);
    }
    int EpollFunc::setNonBlock( int m_sock )
    {
            int old_option = fcntl(m_sock, F_GETFL);
            int new_option = old_option | O_NONBLOCK;
            fcntl(m_sock, F_SETFL, new_option);
            return old_option;
    }
}