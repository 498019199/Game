#ifndef STX_WINDOWS_SOCKET_H
#define STX_WINDOWS_SOCKET_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <WinSock.h>
#include "../Container/var_type.h"
#pragma comment(lib,"ws2_32.lib")
typedef SOCKET socket_t;
#define STX_SOCKET_ERROR SOCKET_ERROR


//初始化通信
inline bool Port_Internetinit()
{
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return false;
    }

    return true;
}

//关闭通信
inline void Port_Internetclose()
{
    WSACleanup();
}

// 创建socket操作，建立流式套接字，返回套接字号sockSrv   
// SOCKET socket(int af, int type, int protocol);   
// 第一个参数，指定地址簇(TCP/IP只能是AF_INET，也可写成PF_INET)   
// 第二个，选择套接字的类型(流式套接字)，第三个，特定地址家族相关协议（0为自动）   
inline socket_t Port_SocketTCP(socket_t *fd)
{
    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == *fd)
    {
        return false;
    }

    return true;
}

//关闭socket
inline void Port_SocketClose(socket_t fd)
{
    closesocket(fd);
}


//how = 0 终止读取操作.
//how = 1 终止传送操作
//how = 2 终止读取及传送操作
inline void Port_SocketShutdown(socket_t fd)
{
    shutdown(fd, /*SD_BOTH*/2);
}

inline void Port_SocketShutsend(socket_t fd)
{
    shutdown(fd, /*SD_SEND*/1);
}

// int listen(SOCKET s,  int backLogManager);   
// 第一个参数:指定需要设置的套接字
// 第二个参数:为（等待连接队列的最大长度）  
inline bool Port_SocketListen(socket_t fd, uint32_t nCount)
{
    int res = listen(fd, nCount);
    if (INVALID_SOCKET == res)
    {
        return false;
    }
    return true;
}

// int connect( SOCKET s,  const struct sockaddr* name,  int namelen);   
// 第一个参数：需要进行连接操作的套接字   
// 第二个参数：设定所需要连接的地址信息   
// 第三个参数：地址的长度   
inline bool Port_SocketConnect(socket_t fd, const char* szIP, int nPort)
{
    sockaddr_in addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(szIP);      // 本地回路地址是127.0.0.1;    
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(nPort);
    if (INVALID_SOCKET == connect(fd, (sockaddr*)&addrSrv, sizeof(sockaddr)))
    {
        return true;
    }

    return false;
}

// int bind(SOCKET s, const struct sockaddr* name, int namelen);   
// 第一个参数，指定需要绑定的套接字；   
// 第二个参数，指定该套接字的本地地址信息，该地址结构会随所用的网络协议的不同而不同   
// 第三个参数，指定该网络协议地址的长度   
inline bool Port_SocketBind(socket_t fd, uint32_t nPort, const char* sIP)
{
    sockaddr_in addr;
    ::ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(nPort);
    if ('\0' == sIP[0])
        addr.sin_addr.s_addr = htonl(INADDR_ANY); // 将INADDR_ANY转换为网络字节序，调用 htonl(long型)或htons(整型)   
    else
        addr.sin_addr.s_addr = inet_addr(sIP);

    // 第二参数要强制类型转换  
    if(SOCKET_ERROR == bind(fd, (sockaddr*)&addr, sizeof(sockaddr_in)))
    {
        return false;
    }
    return true;
}

// accept()，接收连接，等待客户端连接   
// SOCKET accept(  SOCKET s,  struct sockaddr* addr,  int* addrlen);   
// 第一个参数，接收一个处于监听状态下的套接字   
// 第二个参数，sockaddr用于保存客户端地址的信息   
// 第三个参数，用于指定这个地址的长度   
// 返回的是向与这个监听状态下的套接字通信的套接字   
inline socket_t Port_SocketAccept(socket_t fd, char* clientaddr, int *clientport)
{
    sockaddr_in  addrClient;
    struct hostent *hp;
    char* haddrp = nullptr;
    int len = sizeof(addrClient);

    socket_t client_fd = accept(fd, (sockaddr*)&addrClient, &len);
    hp = gethostbyaddr((const char*)&addrClient.sin_addr.s_addr
        , sizeof(addrClient.sin_addr.s_addr), AF_INET);
   
    memcpy(clientaddr, inet_ntoa(addrClient.sin_addr), 32);
    *clientport = addrClient.sin_port;
    return client_fd;
}

// send(), 在套接字上发送数据   
// int send( SOCKET s,  const char* buf,  int len,  int flags);   
// 第一个参数，需要发送信息的套接字，   
// 第二个参数，包含了需要被传送的数据，   
// 第三个参数是buffer的数据长度，   
// 第四个参数，一些传送参数的设置 
inline int Port_SocketSend(socket_t fd, const char* szBuffer, uint32_t len, int flags)
{
    int res = send(fd, szBuffer, len, flags);
    uint64_t err = GetLastError();
    if (err != ERROR_IO_PENDING)
    {
        return res;
    }
    return res;
}

// recv(), 在套接字上接收数据   
// int recv(  SOCKET s,  char* buf,  int len,  int flags);   
// 第一个参数，建立连接后的套接字，   
// 第二个参数，接收数据   
// 第三个参数，接收数据的长度，   
// 第四个参数，一些传送参数的设置   
inline int64_t Port_SocketRecv(socket_t fd, char *context, int len, int flat)
{
    int res = recv(fd, context, len, flat);
    uint64_t err = GetLastError();
    if (err != ERROR_IO_PENDING)
    {
        return res;
    }
    return res;
}

inline bool Port_Ioctlsocket(socket_t fd, long cmd, u_long analy)
{
    ioctlsocket(fd, cmd, &analy);
    return true;
}

#endif//STX_WINDOWS_SOCKET_H