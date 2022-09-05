#include"fiberThread.h"
#include<iostream>
#include<queue>

namespace htb
{
    static thread_local ucontext_t mainFiber;   
    static thread_local fiber* this_fiber = nullptr;

    static thread_local size_t total_fibers=0;
    static uint32_t FiberStacksize = 1024*20;

    class MallocStackAllocator{
        public:
            static void* Alloc(size_t size)
            {
                return malloc(size);
            }
            static void Dealloc(void *vp, size_t size)
            {
                return free(vp);
            }
    };
    fiber::fiber( std::function<void( int fd)> cb, Task task):
    m_cb(cb),m_task(task)
    {
        if( total_fibers==0 )   //说明没有主协程 要先创建主协程
           setMainFiber();    
           //创建子协程
        total_fibers++;
        m_httpConnect = false;
        m_stacksize = FiberStacksize;
        m_stack = MallocStackAllocator::Alloc( m_stacksize );
        m_state = READY;
        getcontext(&m_ctx);
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_size = m_stacksize;
        m_ctx.uc_stack.ss_sp = m_stack;
        makecontext(&m_ctx, mainFunc, 0);
    }
    fiber::~fiber()
    {
        total_fibers--;
        MallocStackAllocator::Dealloc(m_stack, m_stacksize);
    }
    void fiber::mainFunc()
    {
        
        this_fiber->m_state = EXEC;
        this_fiber->m_cb(this_fiber->m_task.getFd());
    }
    void fiber::setMainFiber()
    {
        getcontext(&mainFiber);
    }
    void fiber::reset(Task task )   //m_parse给
    {
        m_task = task;
        m_state = HOLD;
        getcontext(&m_ctx);
        m_ctx.uc_link = &mainFiber;
        m_ctx.uc_stack.ss_size = m_stacksize;
        m_ctx.uc_stack.ss_sp = m_stack;
        makecontext(&m_ctx, mainFunc, 0);
    }
    fiber* fiber::getThis()
    {
        return  this;
    }
    void fiber::swapToMain()
    {
        swapcontext(&(this_fiber->m_ctx), &mainFiber);

    }
    void fiber::swapToBranch( fiber::ptr branchFiber)
    {
        this_fiber = branchFiber->getThis();
        this_fiber->m_state = EXEC;
        //std::cout<<"out"<<std::endl;
        swapcontext(&mainFiber, &(branchFiber->m_ctx));
        
    }
    void fiber::yeildToHold()
    {
        this_fiber->m_state = HOLD;
        swapToMain();
    }
    void fiber::yeildToTerm()
    {
        this_fiber->m_state = TERM;
        swapToMain();
    }
    int fiber::getTotalFiber()
    {
        return total_fibers;
    }
 //######################################################//   
    void Task::recv(  int fd, char buff[] )
    {
        memset(buff,'\0', 1024);
        int sum=0;
        int i=0;
        while( true )
        {
            int rt = ::recv( fd, buff, 1024-sum, 0 );
            if( rt == 0 )
            {
                fiber::yeildToTerm();
            }
            else if( rt>0 )
            {
                sum = sum+rt;
                if( sum>3 && buff[sum-4]=='\r'&&buff[sum-3]=='\n'&& buff[sum-2]=='\r'&&buff[sum-1]=='\n')
                {
                    break;
                }
            }
            else
            {
                if( (errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        std::cout<<fd<<"EAGAIN"<<std::endl;
                        i++;
                        if( i<4 )
                            fiber::yeildToHold();
                        else
                            break;
                    }
                fiber::yeildToTerm();
            }
        }
    }

    void Task::parseHttp(int fd )
    {

        char buff[1024];
        Task::recv( fd, buff);


        htb::HttpRequest::ptr httpRq( new htb::HttpRequest());
        httpRq->parse(buff,strlen(buff));
        std::string response = httpRq->response();
        //std::cout<<response<<std::endl;
        //std::cout<<"parse()"<<std::endl;
        const char *s = response.c_str();
        this_fiber->setHttpConnect( httpRq->getClose());

        int sendlen = strlen( s );
        while( true )
        {
            int sum = 0;
            int rt = ::send(fd, s, strlen(s), 0);
            if( rt<0 )
            {
                if( (errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    std::cout<<fd<<"EAGAIN"<<std::endl;
                    fiber::yeildToHold();
                }
                fiber::yeildToTerm();
            }
            else if( rt>0 )
            {
                sum=sum+rt;
                if( sum == sendlen )
                {
                    //std::cout<<"send suc"<<std::endl;
                    fiber::yeildToTerm();
                }
            }
            else if( rt == 0 )
            {
                fiber::yeildToTerm();
            }
        }
        fiber::yeildToTerm();
    }
 //######################################################//   
        FiberPool::FiberPool( int FiberNums )
        {
            pthread_mutex_init(&m_mutex, NULL);
            pthread_mutex_init(&m_fibermutex, NULL);
            pthread_cond_init(&m_cond,NULL);
            m_readyFiberCount = FiberNums;
            m_holdingFiberCount = 0;
            m_stop = false;
            for( int i = 0; i<FiberNums; i++)
            {
                Task mytask(-1,-1);
                fiber::ptr myfiber( new fiber(htb::Task::parseHttp,mytask));
                m_readyFibers.push( myfiber );
            }
        }
        FiberPool::~FiberPool()
        {
            pthread_cond_destroy(&m_cond);
            pthread_mutex_destroy(&m_mutex);
            pthread_mutex_destroy(&m_fibermutex);

        }
        size_t getTotalFibers()
        {
            return total_fibers;
        }
        void FiberPool::start()
        {
            while( true )
            {
                pthread_mutex_lock(&m_mutex); 
                
                while ( m_Mission.size() == 0 )
                {
                    //std::cout<<"wait"<<std::endl;
                    pthread_cond_wait(&m_cond, &m_mutex);
                }
                if( m_stop == true )
                    break;
                
                fiber::ptr myfiber= m_Mission.front();
                m_Mission.pop();
                pthread_mutex_unlock(&m_mutex);

                fiber::swapToBranch( myfiber );
                if( myfiber->getState() == fiber::HOLD )
                {
                    pthread_mutex_lock(&m_mutex);
                    m_Mission.push(myfiber);
                    pthread_mutex_unlock(&m_mutex);
                    
                    pthread_cond_signal(&m_cond);
                }
                else if( myfiber->getState() == fiber::TERM )
                {
                    //if(myfiber->getHttpConnect() == false )
                    close(myfiber->getTask().getFd());
                    pthread_mutex_lock(&m_fibermutex);
                    m_readyFibers.push(myfiber );
                    pthread_mutex_unlock(&m_fibermutex);
                }
            }
        }
        void FiberPool::addTask( Task task )  //从备用协程池中取出一个闲置协程  与task绑定 加入Mission队列
        {
                pthread_mutex_lock(&m_fibermutex);
                fiber::ptr fiber= m_readyFibers.front();
                m_readyFibers.pop();
                fiber->reset( task );  //重置  协程绑定任务
                pthread_mutex_unlock(&m_fibermutex);
                
                
                pthread_mutex_lock(&m_mutex);
                m_Mission.push( fiber );                //放入任务队列
                //std::cout<<"mission number:"<<m_Mission.size()<<std::endl;
                pthread_mutex_unlock(&m_mutex);


                pthread_cond_signal(&m_cond);  //发出通知
        }
        void FiberPool::stop()
        {
                m_stop = true;
                pthread_cond_signal(&m_cond);
        }




    threadpool::threadpool( int threadnum, int fibernumInthread):
    m_threads_nums(threadnum), m_fibersnumsInthread(fibernumInthread), m_balancenum(0)
    {
        for( int i=0; i<threadnum; i++)
        {
            FiberPool::ptr myfiberpool( new FiberPool(m_fibersnumsInthread));
            m_fiberpools.push_back( myfiberpool );
        }
    }
    void threadpool::addTask( Task task)
    {
        m_balancenum = (m_balancenum++)%m_threads_nums;
        //std::cout<<m_balancenum<<"add task"<<std::endl;
        m_fiberpools[m_balancenum]->addTask( task );

    }
    void threadpool::startPool()
    {
        for( int i=0;i<m_threads_nums;i++)
        {
            std::cout<<"create"<<std::endl;
            pthread_t  pid;
            pthread_create(&pid, NULL, threadFunc,m_fiberpools[i].get());
            m_threads.push_back( pid );
        }
    }
    void threadpool::stopPool()
    {
        for( int i=0;i<m_fiberpools.size();i++)
        {
            m_fiberpools[i]->stop();
        }
    }
    threadpool::~threadpool()
    {
        for( int i=0;i<m_threads.size();i++)
        {
            pthread_join(m_threads[i], NULL);
        }
    
    }
    void* threadpool::threadFunc( void*arg )
    {
        FiberPool *thisptr = (FiberPool *)arg;
        std::cout<<"pool start"<<std::endl;
            thisptr->start();
    }
}