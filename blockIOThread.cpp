#include"blockIOThread.h"
#include<iostream>
namespace htb
{
    void BlockIOThread::addTask( BlockIOTask task)
    {
        pthread_mutex_lock(&m_mutex);
        m_tasks.push(task);
        pthread_mutex_unlock(&m_mutex);

        pthread_cond_signal(&m_cond);
    }
    void BlockIOThread::start()
    {
        pthread_create(&m_pid, NULL, threadFunc, this);
    }
    void BlockIOThread::stopThread()
    {
            m_stop == true;
            pthread_cond_signal(&m_cond);
    }
    void* BlockIOThread::threadFunc( void *arg )
    {
        BlockIOThread* thisptr = (BlockIOThread*)arg;
        while( true )
        {
            //std::cout<<thisptr->m_pid<<"wait"<<std::endl;
            pthread_mutex_lock(&thisptr->m_mutex);
            while( thisptr->m_tasks.size() == 0 )
            {
                pthread_cond_wait(&thisptr->m_cond, &thisptr->m_mutex);
                if( thisptr->m_stop == true )
                   break;
            }
            BlockIOTask task = thisptr->m_tasks.front();
            thisptr->m_tasks.pop();
            pthread_mutex_unlock(&thisptr->m_mutex);

            int fd = task.getFd();
            int epollfd = task.getEpollfd();
            
            char buff[1024];
            memset(buff, '\0', 1024);
            int rt = recv( fd, buff, 1023, 0);
            perror("recv:");
            if( rt==0 )
                    std::cout<<"dis"<<std::endl;
            else
            { 
                htb::HttpRequest::ptr httpRq( new htb::HttpRequest());
                httpRq->parse(buff,strlen(buff));
                std::string response = httpRq->response();
                //std::cout<<response<<std::endl;
                //std::cout<<"parse()"<<std::endl;
                const char *s = response.c_str();


                rt = send( fd, s, strlen(s), 0);
                std::cout<<rt<<":"<<s<<std::endl;
            }
            
        }
    }

    void BlocKIOThreadPool::addTask( BlockIOTask task )
    {
        m_balance = (m_balance+1)%m_threadnums;
        std::cout<<"add task to"<<m_balance<<std::endl;;
        m_threads[m_balance]->addTask( task );
    }
    void BlocKIOThreadPool::start()
    {
        for( int i=0;i<m_threadnums;i++)
            m_threads[i]->start();
    }


}