#ifndef _HTTP_H_
#define _HTTP_H_
#include<memory>
#include<string>
#include<sstream>
#include<string.h>
#include<fstream>
namespace htb
{
class responseMsg
{
public:
    static std::string NotFound;
    static std::string BadRequest;
};

enum PARSE_STATE{
    CHECK_HEAD,
    CHECK_LINE,
};
enum Httpmethod
{
    GET,
    HEAD,
    OPTIONS,
};
enum HttpStatus
{
    GOODLINE,
    BADLINE,
    FINISHLINE
};
class HttpRequest
{
    public:
        typedef std::shared_ptr<HttpRequest> ptr;
    public:  //request
        HttpRequest():m_close(false){}
        HttpStatus getLine( std::string& content, int length, int& begin, int& end);
        HttpStatus parseHead( std::string& content, int begin, int end);
        HttpStatus parseLine( std::string& content, int begin, int end);
        HttpStatus parseBody( char* buff, int begin, int end);
        HttpStatus parse(char* buff, int length);

        Httpmethod getMethod()const{return m_method;}
        std::string getVersion()const{return m_version;}
        std::string getPath()const{return m_path;}
        std::string getHost()const {return m_host;}
        bool getClose()const{return m_close;}
        HttpStatus getStatus()const{return m_status;}

    public: //response
        std::string response();
        bool getFileContent();
    private:  //request
        Httpmethod m_method;
        std::string m_version; 
        std::string m_path;

        std::string m_query;
        std::string m_fragment;
        std::string m_body; //请求消息体
        
        std::string m_host;
        bool m_close;   //false 长链接  true短连接

        HttpStatus m_status;

    private: //response
        std::string m_fileContent;
        std::fstream m_flie;

};


}

#endif