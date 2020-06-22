#ifndef __EasyTcpServer_hpp__
#define __EasyTcpServer_hpp__

#include <iostream>
#include <vector>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

#include "MessageHeader.hpp"
#include "CellTimestamp.hpp"

using namespace std;

class ClientSocket
{
private:
    /* data */
    int _sockfd;

#define RECV_BUFF_SIZE 10240
    char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
    int _lastPos = 0;

public:
    ClientSocket(int cs)
    {
        _sockfd = cs;
        memset(_szMsgBuf, 0, RECV_BUFF_SIZE * 10);
        _lastPos = 0;
    }
    ~ClientSocket()
    {
    }

    int sockfd()
    {
        return _sockfd;
    }

    char *msgBuf()
    {
        return _szMsgBuf;
    }

    int getLast()
    {
        return _lastPos;
    }

    void setLastPos(int n)
    {
        _lastPos = n;
    }
};

class EasyTcpServer
{
private:
    /* data */
    int _sock;
    vector<ClientSocket *> _clients;
    CellTimestamp _tTime;
    int _recvCount;

public:
    EasyTcpServer(/* args */)
    {
        _sock = -1;
        _recvCount = 0;
    }

    ~EasyTcpServer()
    {
    }

    int InitSock()
    {
        int on = 1;
        int ret = -1;
        _sock = socket(PF_INET, SOCK_STREAM, 0);
        if (_sock == -1)
        {
            printf("failed to create socket!\n");
            return -1;
        }

        // flags = fcntl(_sock, F_GETFL, 0);
        // fcntl(_sock, F_SETFL, flags | O_NONBLOCK);

        ret = setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (ret == -1)
        {
            printf("failed to set socket options!\n");
            return -1;
        }

        return _sock;
    }

    int Bind(char *ip, int port)
    {
        int ret;
        struct sockaddr_in localaddr;

        localaddr.sin_family = AF_INET;
        localaddr.sin_port = htons(port);

        if (ip)
            localaddr.sin_addr.s_addr = inet_addr(ip);
        else
        {
            localaddr.sin_addr.s_addr = INADDR_ANY;
        }
        bzero(&(localaddr.sin_zero), 8);
        ret = bind(_sock, (const struct sockaddr *)&localaddr,
                   sizeof(struct sockaddr_in));
        if (ret == -1)
        {
            printf("<socket = %d>failed to bind !\n", _sock);
            return -1;
        }
        else
        {
            /* code */
            printf("<socket = %d>bind 成功!\n", _sock);
        }

        return ret;
    }

    int Listen()
    {
        int ret;
        int backlog = 5;
        ret = listen(_sock, backlog);

        if (ret == -1)
        {
            printf("<socket = %d>failed to listen !\n", _sock);
            return -1;
        }
        else
        {
            printf("<socket = %d>listen 成功\n", _sock);
        }

        return ret;
    }

    int Accept()
    {
        // printf("新客户端接入请求\n");
        int accept_fd;
        // FD_CLR(_sock, &rfds);
        struct sockaddr_in remoteaddr;
        socklen_t address_len = sizeof(struct sockaddr);
        accept_fd = accept(_sock, (struct sockaddr *)&remoteaddr, &address_len);
        if (-1 == accept_fd)
        {
            printf("客户端链接失败\n");
            perror("accept:  ");
            return -1;
        }

        // for (unsigned i = 0; i < _clients.size(); i++)
        // {
        //     NewUserJoin nuj;
        //     nuj.socket = _clients[i]->sockfd();
        //     send(nuj.socket, (const void *)&nuj, sizeof(nuj), 0);
        // }
        // printf("链接开始， 群发新用户加入消息： 客户端 ip %s， port %d\n", inet_ntoa(remoteaddr.sin_addr), remoteaddr.sin_port);
        printf("链接开始<socket %d>, \n", accept_fd);
        _clients.push_back(new ClientSocket(accept_fd));

        return 0;
    }

    void Close()
    {
        if (-1 == _sock)
        {
            close(_sock);
        }
    }
    int count = 0;
    int OnRun()
    {
        if (IsRun())
        {
            int ret = -1;
            int max_fd = _sock + 1;
            fd_set rfds;
            fd_set wfds;
            fd_set efds;

            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            FD_ZERO(&efds);

            FD_SET(_sock, &rfds);
            FD_SET(_sock, &wfds);
            FD_SET(_sock, &efds);

            for (size_t n = 0; n < _clients.size(); n++)
            {
                max_fd = max_fd > (_clients[n]->sockfd() + 1) ? max_fd : (_clients[n]->sockfd() + 1);
                FD_SET(_clients[n]->sockfd(), &rfds);
            }

            timeval t = {1, 0};

            ret = select(max_fd, &rfds, &wfds, &efds, &t);

            // printf("select ret %d count %d\n", ret, count++);
            if (ret < 0)
            {
                printf("select error, \n");
                return -1;
            }
            else if (ret == 0)
            {
                // printf("空闲时间处理业务.\n");
            }
            else
            {
                if (FD_ISSET(_sock, &rfds))
                {
                    Accept();
                }
                else
                {
                    /* code */
                    vector<ClientSocket *>::iterator it;

                    for (it = _clients.begin(); it != _clients.end();)
                    {

                        int tmp_fd = (*it)->sockfd();
                        // printf("< 发现事件的scoket=%d> \n", tmp_fd);
                        if (!FD_ISSET(tmp_fd, &rfds))
                        {
                            it++;
                        }
                        else
                        {
                            ret = RecvData(*it);
                            if (-1 == ret)
                            {
                                // struct sockaddr_in remoteaddr;
                                // socklen_t address_len = sizeof(struct sockaddr);
                                // getsockname(tmp_fd, (sockaddr *)&remoteaddr, &address_len);
                                printf("链接断开： <socket=> %d\n", tmp_fd);

                                if (tmp_fd >= 0)
                                    close(tmp_fd);
                                // printf("*it %p\n", *it);
                                delete (*it);

                                // cout<<it<<endl;
                                // if (it != NULL)
                                // {
                                // printf("it %p\n", it);
                                it = _clients.erase(it);
                                printf("清理已断开socket 完成\n");
                                // }
                            }
                            else
                            {
                                it++;
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }

    // #define RECV_BUFF_SIZE 10240
    char _szRecv[RECV_BUFF_SIZE] = {};
    // char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
    // int _lastPos = 0;
    int RecvData(ClientSocket *pClient)
    {
        char errHeader[10] = "";

        sprintf(errHeader, "<socket=%d>", pClient->sockfd());

        int nlen = recv(pClient->sockfd(), (char *)&_szRecv, RECV_BUFF_SIZE, 0);
        // printf("recv len %d\n", nlen);
        if (nlen == 0)
        {
            Close();

            bzero(pClient->msgBuf(), RECV_BUFF_SIZE * 10);
            pClient->setLastPos(0);
            printf("链接断开\n");
            return -1;
        }

        if (-1 == nlen)
        {
            // printf("errno = %d\n", errno);
            if (errno == EINTR)
            {
                perror(errHeader);
                return 0;
            }
            if (errno == EAGAIN)
            {
                perror(errHeader);
                return 0;
            }

            else
            {
                perror(errHeader);
                return -1;
            }
        }

        //将
        memcpy(pClient->msgBuf() + pClient->getLast(), _szRecv, nlen);
        //消息缓冲区
        pClient->setLastPos(pClient->getLast() + nlen);
        // _lastPos += nlen;
        // printf("接受数据的长度;%d data_header len: %d\n", pClient->getLast(), sizeof(DataHeader));
        while (pClient->getLast() >= sizeof(DataHeader))
        {
            struct DataHeader *dh = (DataHeader *)pClient->msgBuf();
            // printf("dh->datalength : %d\n", dh->dataLength);
            if (pClient->getLast() >= dh->dataLength)
            {
                OnNetMsg(dh, pClient->sockfd());
                // 记录处理消息长度
                int nSize = dh->dataLength;
                memcpy(pClient->msgBuf(), pClient->msgBuf() + nSize, pClient->getLast() - nSize);
                //滑动窗口
                pClient->setLastPos(pClient->getLast() - nSize);
            }
            else
            {
                //剩余数据长度不够一条完整的消息
                break;
            }
        }

        return 0;
    }

    virtual int OnNetMsg(DataHeader *dh, int cs)
    {
        _recvCount++;
        auto t1 = _tTime.getElapsedSecond();
        if (t1 >= 1.0){
            printf("time <%lf>,recvcount <%d>\n", t1, _recvCount);
            _tTime.update();
            _recvCount = 0;
        }

        switch (dh->cmd)
        {
        case CMD_LOGIN:
        {
            Login *login = (Login *)dh;
            // printf("login name :%s pwd: %s\n", login->userName, login->password);

            LoginResult lir;
            send(cs, (const void *)&lir, sizeof(lir), 0);
        }
        break;

        case CMD_LOGOUT:
        {
            // Logout logout;
            // recv(accept_fd, (char *)&logout + dhLen, logout.dataLength - dhLen, 0);
            printf("logout\n");

            LogoutResult lor;

            send(cs, (const void *)&lor, sizeof(lor), 0);
        }
        break;
        default:
        {
            // recv(accept_fd, (char *)&buf, 1024, 0);
            printf("错误的命令\n");
            Error err;
            send(cs, (const void *)&err, sizeof(err), 0);
        }
        break;
        }
        return 0;
    }

    int SendData(DataHeader *dh, int cs)
    {
        if (IsRun() && dh)
            return send(cs, (const void *)dh, dh->dataLength, 0);
        return -1;
    }

    bool IsRun()
    {
        return -1 != _sock;
    }
};

#endif
