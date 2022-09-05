#ifndef _FIBERTHREAD_H_
#define _FIBERTHREAD_H_
#include<ucontext.h>
#include<pthread.h>
#include<memory>
#include<functional>
#include<queue>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<string>
#include<fcntl.h>
#include"epoll.h"
#include<vector>
#include"http.h"
namespace htb
{
    class Task   //非阻塞 读写 解析
    {
        public:
            typedef std::shared_ptr<Task> ptr;
        public:
            Task( int fd, int epollfd ):m_fd(fd),m_epollfd(epollfd){};
            ~Task(){}
            int getFd()const{return m_fd;}
            int getEpollFd()const{return m_epollfd;}
        public:
            static void send( int fd);
            static void recv( int fd, char buff[]);
            static void parseHttp(int fd);
        private:
            int m_fd;
            int m_epollfd;
    };
    class fiber:public std::enable_shared_from_this<fiber>
    {
        public:
            typedef std::shared_ptr<fiber> ptr;
            enum STATE
            {
                READY,    //初始化
                EXEC,     //正在运行
                HOLD,     //在等待队列中
                TERM      //运行结束
            };
        public:
            fiber(std::function<void( int fd )> cb, Task task);
            ~fiber();
            fiber* getThis();
            STATE getState()const{return m_state;} 
            void reset(Task task);
            void setFiberCb(std::function<void(int fd)> cb){m_cb=cb;}
            Task getTask()const{return m_task;}
            bool getHttpConnect()const{return m_httpConnect;}
            void setHttpConnect(bool httpConnect ){ m_httpConnect = httpConnect;}
        public:  
            static void mainFunc();
            static void swapToMain();
            static void swapToBranch( fiber::ptr branchFiber);
            static void yeildToHold();
            static void yeildToTerm();
            static void setMainFiber();
            static int getTotalFiber();
        private:
            ucontext_t m_ctx;
            uint32_t m_stacksize;
            void* m_stack;
            STATE m_state;

            std::function<void(int fd)> m_cb;
            Task m_task;
            bool m_httpConnect;


    };

    class FiberPool
    {
        public:
            typedef std::shared_ptr<FiberPool> ptr;
        public:
            FiberPool( int FiberNums );
            ~FiberPool();
            void start();
            void addTask( Task task );
            void stop();
            void removefd( int fdnums );
            size_t getTotalFibers();
        private:
            pthread_mutex_t m_mutex;
            pthread_mutex_t m_fibermutex;
            pthread_cond_t m_cond;
            size_t m_holdingFiberCount;
            size_t m_readyFiberCount;
            bool m_stop;
            std::queue<fiber::ptr> m_Mission;
            std::queue<fiber::ptr> m_readyFibers;
    };

    class threadpool
    {
        public:
            typedef std::shared_ptr<threadpool> ptr;
        public:
            threadpool( int threadnum, int fibernumInthread);
            void addTask( Task Task);
            void startPool();
            void stopPool();
            int balance();
            ~threadpool();
            static void* threadFunc( void*arg );
        private:
            std::vector<pthread_t> m_threads;
            std::vector<FiberPool::ptr> m_fiberpools;
            int m_balancenum;
            int m_threads_nums;
            int m_fibersnumsInthread;

    };

}
#endif