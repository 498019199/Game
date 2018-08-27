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


//��ʼ��ͨ��
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

//�ر�ͨ��
inline void Port_Internetclose()
{
    WSACleanup();
}

// ����socket������������ʽ�׽��֣������׽��ֺ�sockSrv   
// SOCKET socket(int af, int type, int protocol);   
// ��һ��������ָ����ַ��(TCP/IPֻ����AF_INET��Ҳ��д��PF_INET)   
// �ڶ�����ѡ���׽��ֵ�����(��ʽ�׽���)�����������ض���ַ�������Э�飨0Ϊ�Զ���   
inline socket_t Port_SocketTCP(socket_t *fd)
{
    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == *fd)
    {
        return false;
    }

    return true;
}

//�ر�socket
inline void Port_SocketClose(socket_t fd)
{
    closesocket(fd);
}


//how = 0 ��ֹ��ȡ����.
//how = 1 ��ֹ���Ͳ���
//how = 2 ��ֹ��ȡ�����Ͳ���
inline void Port_SocketShutdown(socket_t fd)
{
    shutdown(fd, /*SD_BOTH*/2);
}

inline void Port_SocketShutsend(socket_t fd)
{
    shutdown(fd, /*SD_SEND*/1);
}

// int listen(SOCKET s,  int backLogManager);   
// ��һ������:ָ����Ҫ���õ��׽���
// �ڶ�������:Ϊ���ȴ����Ӷ��е���󳤶ȣ�  
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
// ��һ����������Ҫ�������Ӳ������׽���   
// �ڶ����������趨����Ҫ���ӵĵ�ַ��Ϣ   
// ��������������ַ�ĳ���   
inline bool Port_SocketConnect(socket_t fd, const char* szIP, int nPort)
{
    sockaddr_in addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(szIP);      // ���ػ�·��ַ��127.0.0.1;    
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(nPort);
    if (INVALID_SOCKET == connect(fd, (sockaddr*)&addrSrv, sizeof(sockaddr)))
    {
        return true;
    }

    return false;
}

// int bind(SOCKET s, const struct sockaddr* name, int namelen);   
// ��һ��������ָ����Ҫ�󶨵��׽��֣�   
// �ڶ���������ָ�����׽��ֵı��ص�ַ��Ϣ���õ�ַ�ṹ�������õ�����Э��Ĳ�ͬ����ͬ   
// ������������ָ��������Э���ַ�ĳ���   
inline bool Port_SocketBind(socket_t fd, uint32_t nPort, const char* sIP)
{
    sockaddr_in addr;
    ::ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(nPort);
    if ('\0' == sIP[0])
        addr.sin_addr.s_addr = htonl(INADDR_ANY); // ��INADDR_ANYת��Ϊ�����ֽ��򣬵��� htonl(long��)��htons(����)   
    else
        addr.sin_addr.s_addr = inet_addr(sIP);

    // �ڶ�����Ҫǿ������ת��  
    if(SOCKET_ERROR == bind(fd, (sockaddr*)&addr, sizeof(sockaddr_in)))
    {
        return false;
    }
    return true;
}

// accept()���������ӣ��ȴ��ͻ�������   
// SOCKET accept(  SOCKET s,  struct sockaddr* addr,  int* addrlen);   
// ��һ������������һ�����ڼ���״̬�µ��׽���   
// �ڶ���������sockaddr���ڱ���ͻ��˵�ַ����Ϣ   
// ����������������ָ�������ַ�ĳ���   
// ���ص��������������״̬�µ��׽���ͨ�ŵ��׽���   
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

// send(), ���׽����Ϸ�������   
// int send( SOCKET s,  const char* buf,  int len,  int flags);   
// ��һ����������Ҫ������Ϣ���׽��֣�   
// �ڶ�����������������Ҫ�����͵����ݣ�   
// ������������buffer�����ݳ��ȣ�   
// ���ĸ�������һЩ���Ͳ��������� 
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

// recv(), ���׽����Ͻ�������   
// int recv(  SOCKET s,  char* buf,  int len,  int flags);   
// ��һ���������������Ӻ���׽��֣�   
// �ڶ�����������������   
// �������������������ݵĳ��ȣ�   
// ���ĸ�������һЩ���Ͳ���������   
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