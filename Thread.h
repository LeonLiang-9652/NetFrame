#ifndef _THREAD_H_
#define _THREAD_H_
#include<pthread.h>
#include<functional>
#include<memory>
#include<queue>
#include<string>
namespace htb
{
    class Mutex
    {
        private:
            pthread_mutex_t m_mutex;
        public:
            Mutex();
            ~Mutex();
            void lock();
            void unlock();
            bool trylock();
            pthread_mutex_t GetMutex() const { return m_mutex;}
    };

    class Condition
    {
        private:
            pthread_cond_t m_condition;
        public:
            Condition();
            ~Condition();
            void Signal();
            void SignalAll();
            void wait(pthread_mutex_t mutex);
            void TimeWait(pthread_mutex_t mutex, int sec);
        
    };

    class Thread
    {
        private:
            pthread_t m_thread;
        public:
            Thread();
            ~Thread();
    };

    
 

}
#endif