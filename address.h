#ifndef _ADDRESS_H_
#define _ADDRESS_H_
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<memory>
#include<string>
#include<fcntl.h>
namespace htb
{
    class Address
    {
        public:
            typedef std::shared_ptr<Address> ptr;
        public:
            Address(){}
            virtual ~Address(){}
            virtual int GetPort()=0;
            virtual void SetPort(int port)=0;
            virtual std::string GetIP()=0;
            virtual void SetIP( std::string IP)=0;
    };

    class IPv4Address : public Address
    {
        public:
            typedef std::shared_ptr<IPv4Address> ptr;
        public:
            IPv4Address(){}
            IPv4Address(std::string ip, int port);
            ~IPv4Address();    
            int GetPort();
            void SetPort(int port);
            std::string GetIP();
            void SetIP( std::string IP);

            sockaddr_in m_address;
            socklen_t m_addlenghth;
        private:
            int m_port;
            std::string m_ip;
    };

    class IPv6Address : public Address
    {
        public:
            typedef std::shared_ptr<IPv6Address> ptr;
        public:
            IPv6Address(std::string ip, int port);
            ~IPv6Address();    
            int GetPort();
            void SetPort(int port);
            std::string GetIP();
            void SetIP( std::string IP);

            sockaddr_in6 m_address;
        private:
            int m_port;
            std::string m_ip;
    };

    class IPv4Socket
    {
        public:
            typedef std::shared_ptr<IPv4Socket> ptr;
        public:
            IPv4Socket(int type);  // type tcp SOCK_STREAM UDP SOCK_DGRAM
            ~IPv4Socket();
            void SetSendTimeout( int sec );
            int GetSendTimeout();

            void SetRecvTimeout( int sec );
            int GetRecvTimeout();
            void SocketInit( int socket, int type, IPv4Address::ptr address );

            IPv4Socket::ptr accept();
            int accept2();
            bool connect( IPv4Address::ptr address );
            bool bind( IPv4Address::ptr address );
            bool listen( );
            void close();
            bool isConnected()const{return m_isconnect;};
            int setNonBlock();


            size_t TcpSend( char* buf, size_t len );
            size_t TcpRecv( char* buf, size_t len );
            size_t UdpSend( char* buf, size_t len, IPv4Address::ptr address);
            size_t UdpRecv( char* buf, size_t len, IPv4Address::ptr address);

            int getSocket() const{return m_sock;};
            
        private:
            int m_sock;
            int m_family; // ipv4 PF_INet   ipv6 PF_INET6
            int m_type;  // tcp SOCK_STREAM    udp SOCK_DGRAM
            int m_protocol; //通常为0
            bool m_isconnect;


            IPv4Address::ptr m_address;
    };
}
#endif
