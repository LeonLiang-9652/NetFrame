#ifndef _LOG_H_
#define _LOG_H_
#include"Thread.h"
#include<map>
#include<string>
#include<memory>
#include<sstream>
#include<fstream>
#include<functional>
#include<iostream>
#include<list>
namespace htb
{   
    enum LEVEL
    {
        UNKNOW=0,
        DEBUG=1,
        INFO=2,
        WARN=3,
        ERROR=4,
        FATAL=5
    };

    class LogEvent
    {
        public:
            LogEvent(LEVEL level, std::string content, pthread_t threadId, int fiberId);
            static std::string Level2String( LEVEL level );
            void refresh();
            int getHour()const{return m_hours;}
            int getMin()const{return m_min;}
            int getSec()const{return m_sec;}
            int getFiberId()const{return m_fiberId;}
            std::string getCol()const{return m_col;}
            std::string getDate()const{return m_date;}
            std::string getContent()const{return m_content;};
            pthread_t getThreadId()const{return m_threadId;}
            std::string getLevel(){return Level2String(m_level);}
            std::string getFile()const{return m_file;}

            void setCol( std::string col){m_col = col;}
            void setFile( std::string file){m_file = file;}
            void setdate( std::string date){m_date = date;}
            void setLevel( LEVEL level){m_level = level;}

        public:
            typedef std::shared_ptr<LogEvent> ptr;

        private:
            int m_hours;
            int m_min;
            int m_sec;
            int m_fiberId;
            std::string m_col;
            std::string m_file;
            std::string m_date;
            std::string m_content;
            pthread_t m_threadId;
            LEVEL m_level;

    };
    class LogFormatter
    {
        //%m 是消息体， %p 日志级别 %r 启动后时间 %c日志名称 %n回车 %t线程 %d时间 %f 文件名 %l行号
        
        public:
            typedef std::shared_ptr<LogFormatter> ptr;
        public:
            LogFormatter(std::string form ):m_form(form){}
            ~LogFormatter(){};
            std::string getItem( LogEvent::ptr event, std::string item );
            void init( LogEvent::ptr event);
            std::string getContent()const{return m_content;}
        private:
            std::string m_content;  //消息
            std::string m_form; //格式
            
    };

    class LogAppender
    {
        public:
            typedef std::shared_ptr<LogAppender> ptr;
            LogAppender(){ m_level = LEVEL::FATAL;}
            virtual ~LogAppender(){}
            virtual void run( std::string in )=0;
            virtual void end( )=0;
        private:
            LEVEL m_level;  //要求输出的等级
    };


    class StdAppender:public LogAppender
    {
        public: 
            typedef std::shared_ptr<StdAppender> ptr;
        public:
            StdAppender(){}
            ~StdAppender(){}
            void run( std::string in ) override;
            void end() override;
            
    };

    class FileAppender:public LogAppender
    {
        public:
            typedef std::shared_ptr<FileAppender> ptr;
        public:
            FileAppender(std::string file);
            ~FileAppender();
            void run( std::string in ) override;
            void end( ) override;
        private:
            std::fstream m_f;
            std::string m_fileName;
    };

    class buff
    {
        public:
            typedef std::shared_ptr<buff> ptr;
            enum STATE
            {
                EMPTY,
                FULL
            };
        public:
            buff( int maxsize ):m_maxsize(maxsize),m_state(EMPTY){ }
            STATE getState( )const{return m_state;};
            void setSTate( STATE state){ m_state = state;}
            void addBuff( buff* m_buff);  //前插
            std::vector<std::string> m_contents;
        private:
            int m_maxsize;
            STATE m_state;
    };

    class LogMQ   //多生产者------单消费者
    {
        public: 
            typedef std::shared_ptr<LogMQ> ptr;
        public:
            LogMQ(std::string file, int nums_buff, int maxsize);
            void addBuff( );
            ~LogMQ();
            void produce( std::string content );
            void consume( int sec);
            static void *ThreadFunc( void* m_this );
        private:
            pthread_mutex_t m_mutex;
            pthread_mutex_t m_mutex2;
            pthread_cond_t m_cond;
            std::vector<buff> m_buffs;
            int m_maxsize;   //每个缓存最大存多少
            int m_cur;       //当前写得内存 index
            int m_consume;     //当前持久化得内存index
            int m_nums_buff; //多少个缓存

            std::fstream m_file;
            std::string m_filename;
            pthread_t m_threadId;

    };
    class AsyAppender:public LogAppender
    {
        public:
            typedef std::shared_ptr<AsyAppender> ptr;
        public:
            std::string getContent( ){return m_content;}
            AsyAppender( LogMQ::ptr mq):m_content(""),m_mq(mq){}
            ~AsyAppender(){}
            void run( std::string in ) override;
            void end( )override;
        private:
            std::string m_content;
            LogMQ::ptr m_mq;

    };
    class Logger
    {
        public:
            Logger(LEVEL level, LogAppender::ptr appender, LogFormatter::ptr format, int fiberId, pthread_t threadid);
            Logger(LEVEL level, LogAppender::ptr appender, LogFormatter::ptr format);
            ~Logger();

            Logger& operator<<( std::string in );
            Logger& operator<<( std::ostream&(*op)(std::ostream&));
            Logger& log( std::string date, std::string file, std::string col, LEVEL level);

        private:
            LogEvent::ptr m_event;
            LogAppender::ptr m_appender;
            LogFormatter::ptr m_format;
            bool m_first;
    };

    class AsyLogger
    {
        public:
            AsyLogger(std::string format, LogMQ::ptr mq ):m_mq(mq),m_format(format)
            {
                m_prefix = "";
                m_content = " ";
            };
            void gettime();
            void parseFormat();
            void makePre(  std::string date, std::string file, std::string col, LEVEL level );
            AsyLogger& log( std::string date, std::string file, std::string col, LEVEL level );
            AsyLogger& operator<<( std::string in );
            AsyLogger& operator<<( std::ostream&(*op)(std::ostream&));
        private:
            std::string m_format;
            
            int m_sec;
            int m_min;
            int m_hour;
            std::string m_time;
            std::string m_level;
            std::string m_date;
            std::string m_filename;
            std::string m_colum;
            std::string m_prefix;

            std::string m_content;
            LogMQ::ptr m_mq;
    };
}
#endif