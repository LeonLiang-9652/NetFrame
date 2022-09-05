#include"Thread.h"
#include<time.h>
namespace htb
{
    Mutex::Mutex()
    {
        pthread_mutex_init( &m_mutex, NULL );
    }
    Mutex::~Mutex()
    {
        pthread_mutex_destroy(&m_mutex );
    }
    void Mutex::lock()
    {
        pthread_mutex_lock(&m_mutex);
    }
    void Mutex::unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }
    bool Mutex::trylock()
    {
        int rt = pthread_mutex_trylock(&m_mutex);
        if( rt == 0 )
         return true;
        else
         return false;
    }

    Condition::Condition()
    {
        pthread_cond_init(&m_condition, NULL);
    }
    Condition::~Condition()
    {
        pthread_cond_destroy(&m_condition);
    }
    void Condition::Signal()
    {
        pthread_cond_signal(&m_condition);
    }
    void Condition::SignalAll()
    {
        pthread_cond_broadcast(&m_condition);
    }
    void Condition::wait(pthread_mutex_t mutex)
    {
        pthread_cond_wait(&m_condition, &mutex);
    }
    void Condition::TimeWait(pthread_mutex_t mutex, int sec)
    {
        timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec = time.tv_sec+sec;
        pthread_cond_timedwait(&m_condition, &mutex, &time);
    }
}