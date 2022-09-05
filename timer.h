#ifndef _TIMER_H_
#define _TIMER_H_
#include<time.h>
#include<iostream>
#include<queue>
namespace htb
{
    class timernode
    {
        public:
            timernode(){}
            ~timernode(){}
            int getFd()const{return m_fd;}
            void setExpire( int sec );
            bool operator<(const timernode& b)const
            {
                return m_expire>b.m_expire;
            }
            bool operator()(const timernode& b )const
            {
                return m_expire>b.m_expire;  //小顶
            }
        private:
            time_t m_expire;
            int m_fd;

    };

    class timer
    {
        public:
            timer(){}
            ~timer(){}
            bool addTimeEvent();
            void tickle();
        private:
            pthread_mutex_t m_mutex;
            std::priority_queue<timernode> m_q;
            static std::vector<bool> m_used;

    };
}
#endif