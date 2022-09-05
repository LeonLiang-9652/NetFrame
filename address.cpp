#include"address.h"
#include<errno.h>
#include<iostream>
namespace htb
{
        IPv4Address::IPv4Address(std::string ip, int port)
        {
            m_port = port;
            m_ip = ip;
            
            const char* ipc = ip.c_str();
            bzero(&m_address, sizeof(m_address));
            m_address.sin_family = AF_INET;
            inet_pton( AF_INET, ipc, &m_address.sin_addr);
            m_address.sin_port = htons( port );
            m_addlenghth = sizeof( m_address);

        }
        IPv4Address::~IPv4Address()
        {
            m_ip="";
            m_port=-1;
        }    
        int IPv4Address::GetPort()
        {
            if( m_port!=htons(m_address.sin_port))
                m_port = htons(m_address.sin_port);
            return htons(m_address.sin_port);
        }
        void IPv4Address::SetPort(int port)
        {
            m_port = port;
            m_address.sin_port = htons( port );
        }
        std::string IPv4Address::GetIP()
        {
            if( m_ip!=inet_ntoa(m_address.sin_addr))
                m_ip = inet_ntoa(m_address.sin_addr);
            return inet_ntoa(m_address.sin_addr);
        }
        void IPv4Address::SetIP( std::string IP)
        {
            m_ip = IP;
            const char* ipc = IP.c_str();
            inet_pton( AF_INET, ipc, &m_address.sin_addr);
        }
        IPv6Address::IPv6Address(std::string ip, int port)
        {
            m_ip = ip;
            m_port = port;
            const char* ipc = ip.c_str();
            bzero(&m_address, sizeof(m_address));
            m_address.sin6_family = AF_INET6;
            inet_pton( AF_INET6, ipc, &m_address.sin6_addr);
            m_address.sin6_port = htons( port );
        }
        IPv6Address::~IPv6Address()
        {

        }    
        int IPv6Address::GetPort()
        {
            return m_port;
        }
        void IPv6Address::SetPort(int port)
        {
            m_port=port;
            m_address.sin6_port = htons( port );
        }
        std::string IPv6Address::GetIP()
        {
            return m_ip;
        }
        void IPv6Address::SetIP( std::string IP)
        {
            m_ip = IP;
            const char* ipc = IP.c_str();
            inet_pton( AF_INET6, ipc, &m_address.sin6_addr);
        }
            //PF_INET
        IPv4Socket::IPv4Socket(int type)
        {
            m_family = PF_INET;
            m_type = type;
            m_protocol = 0;
            m_isconnect = false;
            m_sock=socket(m_family, m_type,m_protocol);
        }
        IPv4Socket::~IPv4Socket()
        {

        }
        void IPv4Socket::SetSendTimeout( int sec )
        {

        }
        int IPv4Socket::GetSendTimeout()
        {

        }

        void IPv4Socket::SetRecvTimeout( int sec )
        {

        }
        int IPv4Socket::GetRecvTimeout()
        {

        }
        void IPv4Socket::SocketInit( int socket, int type, IPv4Address::ptr address)
        {
                m_protocol = PF_INET;
                m_type = type;
                m_protocol = 0;
                m_sock = socket;
                m_address = address;
                m_isconnect = true;
        }

        IPv4Socket::ptr IPv4Socket::accept()
        {
            IPv4Socket::ptr NewSock(new IPv4Socket(m_type));
            IPv4Address::ptr NewAddress( new IPv4Address());
            int socket = ::accept( m_sock, (sockaddr* )&(NewAddress->m_address), &(NewAddress->m_addlenghth));
            if( socket == -1 )
            {
                perror("accept");
                return  nullptr;
            }
            NewSock->SocketInit( socket, m_type, NewAddress);
            return NewSock;
        }
        int IPv4Socket::accept2()
        {
            IPv4Address::ptr NewAddress( new IPv4Address());
            int socket = ::accept( m_sock, (sockaddr* )&(NewAddress->m_address), &(NewAddress->m_addlenghth));
            return socket;
        }
        bool IPv4Socket::bind( IPv4Address::ptr address )
        {
            int rt = ::bind( m_sock, (sockaddr*)&(address->m_address), sizeof(address->m_address));
            if( rt == -1 )
            {
                perror("bind");
                return false;
            }
            else
            return true;
        }
        bool IPv4Socket::connect( IPv4Address::ptr address )
        {
            int rt = ::connect( m_sock, (sockaddr*)&(address->m_address), sizeof(address->m_address));
            if( rt<0 )
            {
                perror("connect");
                return false;
            }
            else
            {
                m_isconnect = true;
            return true;
            }
        }
        bool IPv4Socket::listen( )
        {
            int rt = ::listen( m_sock, 20);
        }
        void IPv4Socket::close()
        {
            ::close(m_sock);
            m_sock=-1;
        }
        int IPv4Socket::setNonBlock()
        {
            int old_option = fcntl(m_sock, F_GETFL);
            int new_option = old_option | O_NONBLOCK;
            fcntl(m_sock, F_SETFL, new_option);
            return old_option;
        }
        size_t IPv4Socket::TcpSend( char* buf, size_t len )
        {
            return ::send( m_sock, buf, len, 0);
        }
        size_t IPv4Socket::TcpRecv(char* buf, size_t len)
        {
            return ::recv( m_sock, buf, len, 0);
        }
        size_t IPv4Socket::UdpSend(char* buf, size_t len, IPv4Address::ptr address)
        {
            return ::sendto(m_sock, buf, len, 0,(sockaddr*)&(address->m_address), address->m_addlenghth);
        }
        size_t IPv4Socket::UdpRecv(char* buf, size_t len, IPv4Address::ptr address)
        {
            return ::recvfrom(m_sock, buf, len, 0,(sockaddr*)&(address->m_address), &address->m_addlenghth);
        } 
}