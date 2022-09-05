#include"log.h"
namespace htb
{   
    LogEvent::LogEvent(LEVEL level, std::string content="", pthread_t threadId = -1, int fiberId=-1)
    :m_fiberId(fiberId),m_content(content),
    m_threadId(threadId),m_level(level)
    {
        time_t t = time(NULL);
        tm *now_time = localtime(&t);
        m_hours = 0;
        m_min = 0;
        m_sec = 0;
        m_col ="";
        m_date="";
        m_file="";
    }
    void LogEvent::refresh()
    {
        time_t t = time(NULL);
        tm *now_time = localtime(&t);
        m_hours = now_time->tm_hour;
        m_min = now_time->tm_min;
        m_sec = now_time->tm_sec;
    }
    std::string LogEvent::Level2String( LEVEL level )
    {
        if( level == UNKNOW )
            return "UNKOWN";
        if( level == DEBUG )
            return "DEBUG";
        if( level ==  INFO)
            return "INFO";
        if( level == WARN )
            return "WARN";
        if( level == ERROR )
            return "ERROR";
        if( level == FATAL )
            return "FATAL";
        
    }
    //%m 是消息体， %p 日志级别 %r 启动后时间 %c日志名称 %n回车 %t线程 %d时间 %f 文件名 %l行号
    std::string LogFormatter::getItem( LogEvent::ptr event, std::string item )
    {
        if( item == "%m")
            return event->getContent();
        if( item == "%p")
            return "["+event->getLevel()+"]\t";
        if( item =="%r")
            {
                int hour = event->getHour();
                int min = event->getMin();
                int sec = event->getSec();
                std::string s ="["+std::to_string(hour)+":"+std::to_string(min)+":"+std::to_string(sec)+"]\t";
                return s;
            }
        if( item =="%n")
            return "\n";
        if( item =="%t")
            if( event->getThreadId()!=-1)
                return "["+std::to_string(event->getThreadId())+"]\t";
        if( item == "%f")
            if( event->getFiberId()!=-1)
                return "["+std::to_string(event->getFiberId())+"]\t";
        if( item =="%l")
            return "[line:"+event->getCol()+"]\t";
        if( item =="%d")
            return "["+event->getDate()+"]\t";
        if( item =="%a")
            return "["+event->getFile()+"]\t";
        return "";

    }
    void LogFormatter::init( LogEvent::ptr event)
    {
        m_content="";
        int i=0;
        while( i<m_form.length() )
        {
            if( m_form[i] == '[')
            {
                //std::cout<<getItem(event, m_form.substr(i+1,2))<<std::endl;
                m_content=m_content+getItem(event, m_form.substr(i+1,2));
                i = i+3;
            }
            else
            i++;
        }

    }


    void StdAppender::run( std::string in )
    {
        std::cout<<in;
    }
    void StdAppender::end()
    {
        std::cout<<std::endl;
    }

    FileAppender::FileAppender(std::string file):m_fileName(file)
    {
        m_f.open(file, std::ios::out| std::ios::app);
    }
    FileAppender::~FileAppender()
    {
        m_f.close();
    }
    void FileAppender::run( std::string in )
    {
        if(m_f.is_open()!=true)
            m_f.open(m_fileName, std::ios::out| std::ios::app);
        m_f<<in;
    }
    void FileAppender::end()
    {
        m_f<<std::endl;
    }
    LogMQ::LogMQ(std::string file,int nums_buff, int maxsize):
    m_maxsize(maxsize),m_nums_buff(nums_buff),m_filename(file)
    {
        m_cur = 0;
        m_consume=0;
        for( int i=0;i<nums_buff;i++)
        {
            m_buffs.push_back(buff(maxsize));
        }
        pthread_mutex_init(&m_mutex2, NULL);
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_cond, NULL);
        pthread_create(&m_threadId, NULL, ThreadFunc, this);
    }
    LogMQ::~LogMQ()
    {
        pthread_mutex_destroy(&m_mutex2);
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
        m_file.close();
        pthread_join(m_threadId, NULL);
    }
    void LogMQ::produce( std::string content )
    {
        pthread_mutex_lock(&m_mutex);
        //修改状态
        if( m_buffs[m_cur].getState() == buff::EMPTY )
        {
            m_buffs[m_cur].setSTate( buff::FULL);
            m_buffs[m_cur].m_contents.push_back(content );
        }

        else if( m_buffs[m_cur].getState() == buff::FULL )
        {
            if( m_buffs[m_cur].m_contents.size() == m_maxsize)   //写满了指向下一个
            {
                pthread_mutex_lock(&m_mutex2);
                m_buffs.insert(m_buffs.begin()+m_cur, buff(m_maxsize));   //说明内存不够
                std::cout<<m_cur<<std::endl;
                m_nums_buff++;
                m_consume=(m_consume+1)%m_nums_buff;
                m_buffs[m_cur].setSTate(buff::FULL);
                std::cout<<m_buffs[m_cur].m_contents.size()<<std::endl;;
                m_buffs[m_cur].m_contents.push_back(content );
                pthread_mutex_unlock(&m_mutex2);
            }
            else  //没满继续写
            {
                m_buffs[m_cur].m_contents.push_back( content );

                if( m_buffs[m_cur].m_contents.size()==m_maxsize )
                    m_cur = (m_cur+1)%m_nums_buff;
            }
        }
        pthread_mutex_unlock(&m_mutex);

    }
    void LogMQ::consume( int sec )
    {
        
        while( true )
        {

            pthread_mutex_lock(&m_mutex);
            while(m_buffs[m_consume].getState()==buff::EMPTY)
            {
                timespec time;
                clock_gettime(CLOCK_REALTIME, &time);
                time.tv_sec = time.tv_sec+sec;
                pthread_cond_timedwait(&m_cond, &m_mutex, &time);
            }
            if( m_cur == m_consume )                        //m_cur和要持久化的是一个  让m_cur指向下一个
                m_cur = (m_cur+1)%m_maxsize;
            
            auto it = m_buffs.begin()+m_consume;
            pthread_mutex_unlock(&m_mutex);
                                                            //m_cur 和 m_consume 不是一个,开始持久化
            pthread_mutex_lock(&m_mutex2);
            //std::cout<<"begin"<<std::endl;
            if( m_file.is_open() == false )
            {
                m_file.open(m_filename, std::ios::out| std::ios::app);
                //std::cout<<m_filename<<std::endl;
            }
            //std::cout<<m_buffs[m_consume].m_contents.size()<<std::endl;
            for( int i=0 ;i<it->m_contents.size(); i++)
            {
                //std::cout<<m_file.is_open()<<std::endl;
                m_file<<it->m_contents[i]<<std::endl;
                //std::cout<<m_buffs[m_consume].m_contents[i]<<std::endl;
            }
            it->m_contents.clear();          
            it->setSTate(buff::EMPTY);       
                                                            //持久化完成换下一个
            m_consume = (m_consume+1)%m_nums_buff;
            pthread_mutex_lock(&m_mutex2);
            //std::cout<<"end"<<std::endl;
        }
    }
    void* LogMQ::ThreadFunc( void* m_this )
    {
        LogMQ* ptr = (LogMQ*)m_this;
        ptr->consume(1);
    }

    void AsyAppender::run( std::string in )
    {   
        m_content = m_content+in;
    }
    void AsyAppender::end( )
    {
        //m_mq->produce(m_content);
        m_content.clear();
    }


    Logger::Logger(LEVEL level, LogAppender::ptr appender, LogFormatter::ptr format, int fiberId, pthread_t threadid):
    m_event(std::make_shared<LogEvent>(level,"", threadid, fiberId)),
    m_appender(appender),
    m_format(format), m_first(false)
    {

    }
    Logger::Logger(LEVEL level, LogAppender::ptr appender, LogFormatter::ptr format):
    m_event(std::make_shared<LogEvent>(level,"", -1, -1)),
    m_appender(appender),
    m_format(format), m_first(false)
    {

    }
    Logger::~Logger()
    {

    }
    Logger& Logger::operator<<( std::string in )
    {
        if( m_first==false )
        {
            m_event->refresh();
            m_format->init(m_event);
            //std::cout<<m_format->getContent();
            in = m_format->getContent()+in;
            m_first = true;
        }
        m_appender->run( in );
        return *this;
    }
    Logger& Logger::operator<<( std::ostream&(*op)(std::ostream&))
    {
        m_appender->end();
        m_first = false;
        return *this;
    }
    Logger& Logger::log( std::string date, std::string file, std::string col, LEVEL level)
    {
        m_event->setdate(date);
        m_event->setFile(file);
        m_event->setCol(col);
        m_event->setLevel(level);
        return *this;
    }


    void AsyLogger::gettime()
    {
        time_t t = time(NULL);
        tm *now_time = localtime(&t);
        m_hour = now_time->tm_hour;
        m_min = now_time->tm_min;
        m_sec = now_time->tm_sec;
        m_time = "["+std::to_string(m_hour)+":"+std::to_string(m_min)+":"+std::to_string(m_sec)+"]\t";

    }
//"[%d] [%r] [%p] [%a] [%l]"  %d __date__ rtime  p level a filename l colum
    void AsyLogger::parseFormat()
    {
        m_prefix="";
        for( int i=0;i<m_format.length();i++ )
        {
            if( m_format[i] == 'd')
                m_prefix=m_prefix+m_date;
            else if( m_format[i] == 'r')
                m_prefix=m_prefix+m_time;
            else if( m_format[i] == 'p')
                m_prefix=m_prefix+m_level;
            else if( m_format[i] == 'a')
                m_prefix=m_prefix+m_filename;
            else if( m_format[i] == 'l')
                m_prefix=m_prefix+m_colum;
        }
    }
    void AsyLogger::makePre(  std::string date, std::string file, std::string col, LEVEL level )
    {
        m_date='['+date+"]\t";
        m_filename='['+file+"]\t";
        m_colum = '['+col+"]\t";
        m_level ='['+LogEvent::Level2String(level)+"]\t";
    }
    AsyLogger& AsyLogger::log( std::string date, std::string file, std::string col, LEVEL level )
    {
        makePre(date, file, col, level);
        gettime();
        parseFormat();
        return *this;
    }
    AsyLogger& AsyLogger::operator<<( std::string in )
    {
        m_content = m_content+in;
        return *this;
    }
    AsyLogger& AsyLogger::operator<<( std::ostream&(*op)(std::ostream&))
    {
        m_content = m_prefix+m_content;
        m_mq->produce(m_content);
        m_content.clear();
    }
}