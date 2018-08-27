//#include <iostream>
//#include "../lib/global.h"
//#include "../net/net_work.h"
//#include "../net/http_request.h"
//#include "../net/htpp_response.h"
//
//
//#define LISTENQ 10
//#define LENGTH_MAX 4096
//#define MAXPARAM 2048  
//#define MAXLINE 1028
//NetWork::~NetWork()
//{
//    Release();
//}
//
//bool NetWork::OpenServer(int port)
//{
//    Port_Internetinit();
//    if (!Port_SocketTCP(&m_socket))
//        STX_ASSERT("server socket create fail");
//    if (!Port_SocketBind(m_socket, port, ""))
//        STX_ASSERT("server bind fail");
//    if (!Port_SocketListen(m_socket, LISTENQ))
//        STX_ASSERT("server listen fail");
//    //Port_Ioctlsocket(m_socket, FIONBIO, 1);
//    return true;
//}
//
//bool NetWork::LoopAccept()
//{
//
//    socket_t client_fd = 0;
//    char client_addr[32] = {};
//    int client_port = 0;
//    while (1)
//    {
//        client_fd = Port_SocketAccept(m_socket, client_addr, &client_port);
//        if (client_fd < 0)
//        {
//            LOGER_ERROR("client fd is empty");
//        }
//        char buff[1024] = {};
//        SAFE_PRINTF(buff, "Accepted client IP:[%s], port:[%d]\n", client_addr, client_port);
//        LOGER_INFO(buff);
//
//        char sbuffer[1024] = {};
//        RealRecv(client_fd, sbuffer);
//        if (STRING_EMPTY(sbuffer))
//        {
//            continue;
//        }
//        IContent *pContent = NEW IContent(sbuffer);
//        HttpResponse response(pContent);
//        response.HttpResolving();
//        HttpRequest request(client_fd, pContent);
//       std::string msg = 
//            request.GetHeader(501, "Tiny does not implement this method");
//
//        //要发送给客户的信息  
//        int nSend = Port_SocketSend(client_fd, msg.c_str(), msg.size(), 0);
//        if (STX_SOCKET_ERROR == nSend)
//        {
//            char buff[1024] = {};
//            SAFE_PRINTF(buff, "client IP:[%d], port:[%d]. send() byte: [%d]\n", 
//                client_addr, client_port, nSend);
//            LOGER_ERROR(buff);
//        }
//        delete pContent;
//        Port_SocketClose(client_fd);
//    }
//
//    return true;
//}
//
//void NetWork::client_error(int fd, char* cause, char *errnum, char* shortmsg, char* LogManagermsg)
//{
//    char buf[MAXLINE], body[MAXLINE];
//
//    /* Build the HTTP response body */
//    SPRINTF(body, "<html><title>Tiny Error</title>");
//    SPRINTF(body, "%s<body bgcolor=""ffffff"">\r\n", body);
//    SPRINTF(body, "%s%s: %s\r\n", body, errnum, shortmsg);
//    SPRINTF(body, "%s<p>%s: %s\r\n", body, LogManagermsg, cause);
//    SPRINTF(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);
//    /* Print the HTTP response */
//    SPRINTF(buf, "HTTP/1.1 %s %s\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n", errnum, shortmsg, (int)strlen(body));
//    Port_SocketSend(fd, buf, strlen(buf), 0);
//    Port_SocketSend(fd, body, strlen(body), 0);
//}
//
//
//void NetWork::read_request(void *rp)
//{
//
//}
//
//void NetWork::serve_static(int fd, char* filename, int filesize)
//{
//
//}
//
//void NetWork::serve_dynamic(int fd, char* filename, int filesize)
//{
//
//}
//
//int NetWork::RealSend(socket_t clientfd)
//{
//    int res = 0;
//    return res;
//}
//
//int NetWork::RealRecv(socket_t clientfd, char* sbuffer)
//{
//    int i = 0;
//    char ch = '\0';
//    int64_t n = -1;
//
//    while (i < 1024 - 1 && 0 != n)
//    {
//        n = Port_SocketRecv(clientfd, &ch, 1, 0);
//        if (n > 0)
//        {
//            if ('\r' == ch)
//            {
//                if (n > 0 && ch == '\n')
//                    n = Port_SocketRecv(clientfd, &ch, 1, MSG_PEEK);
//                else
//                    ch = '\r';
//            }
//            sbuffer[i] = ch;
//            i++;
//        }
//    }
//
//    sbuffer[i] = '\0';
//    return i;
//}
//
//void NetWork::Release()
//{
//    //停止接受
//    Port_SocketShutdown(m_socket);
//    //关闭
//    Port_SocketClose(m_socket);
//    //结束通信
//    Port_Internetclose();
//}
//
//
////钩子函数
////回调函数
////心跳函数