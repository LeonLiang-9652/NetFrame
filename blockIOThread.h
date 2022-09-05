#ifndef _BLOCKIOTHREAD_H_
#define _BLOCKIOTHREAD_H_
#include<memory>
#include<vector>
#include<pthread.h>
#include<queue>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<string.h>
#include"http.h"
#include<unistd.h>
#include"epoll.h"
namespace htb
{
    class BlockIOTask
    {
        public:
            BlockIOTask( int fd, int epollfd):m_fd(fd), m_epollfd(epollfd){}
            int getFd()const{return m_fd;}
            int getEpollfd()const{return m_epollfd;}
        private:
            int m_fd;
            int m_epollfd;
    };
    class BlockIOThread
    {
        public: 
            typedef std::shared_ptr<BlockIOThread> ptr;
        public:
            BlockIOThread()
            {
                pthread_mutex_init(&m_mutex, NULL);
                pthread_cond_init(&m_cond, NULL);
                m_stop = false;
            }
            ~BlockIOThread()
            {
                pthread_mutex_destroy(&m_mutex);
                pthread_cond_destroy(&m_cond);
                pthread_join(m_pid, NULL);
            }

            void addTask( BlockIOTask task);
            void start();
            void stopThread();
            static void* threadFunc( void *arg );

        private:
            pthread_t m_pid;
            pthread_mutex_t m_mutex;
            pthread_cond_t m_cond;
            bool m_stop;
            std::queue<BlockIOTask> m_tasks;
                 
        
    };

    class BlocKIOThreadPool
    {
        public:
            typedef std::shared_ptr<BlocKIOThreadPool> ptr;
        public:
            BlocKIOThreadPool( int threadnums ):m_threadnums(threadnums)
            {
                m_balance = 0;
                BlockIOThread::ptr mythread( new BlockIOThread());
                for( int i=0;i<threadnums;i++ )
                    m_threads.push_back( mythread );
            }
            void start();
            void addTask( BlockIOTask task );
            void stoppool();
        private:
            int m_threadnums;
            int m_balance;
            std::vector<BlockIOThread::ptr> m_threads;

    };
}
#endif