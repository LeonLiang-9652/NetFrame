#include"http.h"
#include<iostream>
#include<string.h>
namespace htb
{
        std::string responseMsg::NotFound = "HTTP/1.1 404 Not Found\r\nContent-Length:0\r\n";;
        std::string responseMsg::BadRequest = "HTTP/1.1 400 Bad Request\r\nContent-Length:0\r\n";
        HttpStatus HttpRequest::getLine( std::string& content, int length, int& begin, int& end)
        {
            for( int i=begin;i<length;i++)
            {
                if( i<length-1&&content[i]=='\r'&&content[i+1]=='\n')
                {
                    end = i+1;
                    if( (end-begin+1) == 2)
                        return HttpStatus::FINISHLINE;
                    return HttpStatus::GOODLINE;
                }
            }
            return HttpStatus::BADLINE;
        }
        HttpStatus HttpRequest::parseHead( std::string& content, int begin, int end)
        {
            std::string head = content.substr(begin,end-begin+1);
            //std::cout<<"head: "<<head<<std::endl;
            int space1;
            int space2;
            int flag=0;
            //分割出来，如果没有两个空格 错误的
            for( int i=0;i<head.length();i++ )
            {
                if( head[i]==' ' && flag==0 )
                {
                    space1 = i;
                    flag++;
                }
                else if( head[i]==' ' && flag==1)
                {
                    space2=i;
                    flag++;
                    break;
                }
            }
            if( flag !=2 )    //没有两个空格 格式不对
                return HttpStatus::BADLINE;

            std::string method = head.substr(0,space1);
            m_path = head.substr(space1+1, space2-space1-1);
            std::string version = head.substr(space2+1, end-2-space2);
            if( method == "GET")
                m_method = Httpmethod::GET;
            else if( method == "OPTIONS")
                m_method = Httpmethod::OPTIONS;
            else if( method == "HEAD")
                m_method = Httpmethod::HEAD;
            else
                return HttpStatus::BADLINE;

            if( version == "HTTP/1.1")
                m_version = "HTTP/1.1";
            else
                return HttpStatus::BADLINE;
            return HttpStatus::GOODLINE;
        }
        HttpStatus HttpRequest::parseLine( std::string& content, int begin, int end)
        {
            std::string line = content.substr(begin, end-begin+1);
            int fenhao=-1;
            for( int i=0;i<line.length();i++)
                if( line[i] == ':' )
                {
                    fenhao = i;
                    break;
                }
            if(fenhao == -1)
                return HttpStatus::BADLINE;
            std::string key = line.substr(0, fenhao);
            std::string value = line.substr(fenhao+1, line.length()-fenhao-3);
            if( key == "Connection")
            {
                if( value == "keep-alive")
                {
                    m_close = false;
                }
                else if( value == "close")
                {
                    m_close = true;
                }
                else
                    return HttpStatus::BADLINE;
            }
            else if( key == "Host")
            {
                m_host = value;
            }
            return HttpStatus::GOODLINE; 
        }
        HttpStatus HttpRequest::parseBody( char* buff, int begin, int end)
        {

        }
        HttpStatus HttpRequest::parse(char* buff, int length)
        {
            PARSE_STATE state = PARSE_STATE::CHECK_HEAD;
            HttpStatus Httpstate;
            std::string content(buff);
            //std::cout<<content<<std::endl;
            int begin=0;
            int end=0;
            while( getLine(content, length, begin, end)!=HttpStatus::FINISHLINE)
            {
                switch( state)
                {
                    case PARSE_STATE::CHECK_HEAD:
                    {
                        Httpstate = parseHead(content, begin, end);
                        if( Httpstate == HttpStatus::BADLINE)
                        {
                            m_status = HttpStatus::BADLINE;
                            return HttpStatus::BADLINE;
                        }
                        else
                        {
                            state = PARSE_STATE::CHECK_LINE;
                        }
                        break;
                    }
                    case PARSE_STATE::CHECK_LINE:
                    {
                        Httpstate = parseLine(content, begin,end);
                        if( Httpstate == HttpStatus::BADLINE )
                        {
                            m_status = HttpStatus::BADLINE;
                            return HttpStatus::BADLINE;
                        }
                        break;
                    }
                }
                begin=end+1;
            }
            return HttpStatus::GOODLINE;   
        }
        bool HttpRequest::getFileContent()
        {
            std::string line="";
            m_fileContent = "";
            m_flie.open(m_path, std::ios::in);
            bool open = m_flie.is_open();

            if( open )
            {
                while(!m_flie.eof())
                {
                 m_flie>>line;
                 if( line!="")
                 m_fileContent=m_fileContent+line+'\n';
                }
                return true;
            
            }
            else
             return false;
        }
        std::string HttpRequest::response( )
        {
            std::string response="";
            std::string Connection;   //Connection 是回应行 最后一行
            if( m_close == true )
            {
                Connection="Connection:close\r\n\r\n";
            }
            else
            {
                Connection="Connection:keep-alive\r\n\r\n";
            }
             if( m_status == HttpStatus::BADLINE)
             {
                 response = response+responseMsg::BadRequest+Connection;
                 return response;
             }
            if( m_method == Httpmethod::GET)
            {
                m_fileContent="";
                if( getFileContent() == true )
                {
                    if(m_fileContent.length() == 0)   //文件有内容
                    {              
                        response = "HTTP/1.1 204 No Content\r\nContent-type:text\r\nContent-Length:0\r\n";
                        response=response+Connection;
                        return  response;
                    }
                    else   //文件有内容
                    {
                        response = "HTTP/1.1 200 OK\r\nContent-type:text\r\nContent-Length:";
                        response = response+std::to_string(m_fileContent.length())+"\r\n"+Connection+m_fileContent;
                    }
                }
                else
                {
                    response = responseMsg::NotFound+Connection;
                    return response;
                }
            }
            else if( m_method == Httpmethod::HEAD)
            {
                if( getFileContent() == true )
                {
                    if(m_fileContent.length() == 0)   //文件有内容
                    {              
                        response = "HTTP1.1 204 No Content\r\nContent-type:text\r\n";
                        response=response+Connection;
                        return  response;
                    }
                    else   //文件有内容
                    {
                        response = "HTTP1.1 200 No ok\r\nContent-type:text\r\n";
                        response = response+Connection+m_fileContent;
                    }
                }
                else
                {
                    response = responseMsg::NotFound+Connection;
                    return response;
                }
            }
            else if( m_method == Httpmethod::OPTIONS)
            {
                response = "HTTP1.1 204 No Content\r\nAllow:GET,HEAD,OPTIONS\r\nContent-Length:0\r\n";
                response = response+Connection;
                return response;
            }
            return response;
        }
}