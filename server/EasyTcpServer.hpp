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

using namespace std;

class EasyTcpServer
{
private:
    /* data */
    int _sock;
    vector<int> g_sockets;

public:
    EasyTcpServer(/* args */)
    {
        _sock = -1;
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
        printf("新客户端接入请求\n");
        int accept_fd;
        // FD_CLR(_sock, &rfds);
        struct sockaddr_in remoteaddr;
        socklen_t address_len = sizeof(struct sockaddr);
        accept_fd = accept(_sock, (struct sockaddr *)&remoteaddr, &address_len);
        if (-1 == accept_fd)
        {
            printf("客户端链接失败\n");
        }

        for (unsigned i = 0; i < g_sockets.size(); i++)
        {
            NewUserJoin nuj;
            nuj.socket = g_sockets[i];
            send(g_sockets[i], (const void *)&nuj, sizeof(nuj), 0);
        }
        printf("链接开始， 群发新用户加入消息： 客户端 ip %s， port %d\n", inet_ntoa(remoteaddr.sin_addr), remoteaddr.sin_port);
        g_sockets.push_back(accept_fd);
        return 0;
    }

    void Close()
    {
        if (-1 == _sock)
        {
            close(_sock);
        }
    }

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

            for (size_t n = 0; n < g_sockets.size(); n++)
            {
                max_fd = max_fd > (g_sockets[n] + 1) ? max_fd : (g_sockets[n] + 1);
                FD_SET(g_sockets[n], &rfds);
            }

            timeval t = {1, 0};

            ret = select(max_fd, &rfds, &wfds, &efds, &t);
            if (ret < 0)
            {
                printf("select error, \n");
                return -1;
            }
            else if (ret == 0)
            {
                printf("空闲时间处理业务.\n");
            }
            else
            {
                if (FD_ISSET(_sock, &rfds))
                {
                    Accept();
                }

                vector<int>::iterator it;

                for (it = g_sockets.begin(); it != g_sockets.end();)
                {
                    if (!FD_ISSET(*it, &rfds))
                    {
                        it++;
                    }
                    else
                    {
                        ret = RecvData(*it);
                        if (-1 == ret)
                        {
                            close(*it);
                            struct sockaddr_in remoteaddr;
                            socklen_t address_len = sizeof(struct sockaddr);
                            getsockname(*it, (sockaddr *)&remoteaddr, &address_len);
                            printf("链接断开： 客户端 ip %s， port %d\n", inet_ntoa(remoteaddr.sin_addr), remoteaddr.sin_port);
                            it = g_sockets.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }
                }
            }
        }
        return false;
    }

    int RecvData(int cs)
    {
        struct DataHeader *dh;
        size_t dhLen = sizeof(DataHeader);
        char buf[1024];

        int nlen = recv(cs, (char *)&buf, dhLen, 0);
        if (nlen == 0)
        {
            Close();
            printf("链接断开\n");
            return -1;
        }
        dh = (DataHeader *)buf;

        recv(cs, (char *)&buf + dhLen, dh->dataLength - dhLen, 0);
        OnNetMsg(dh, cs);
        return 0;
    }

    virtual int OnNetMsg(DataHeader *dh, int cs)
    {
        switch (dh->cmd)
        {
        case CMD_LOGIN:
        {
            Login *login = (Login *)dh;
            printf("login name :%s pwd: %s\n", login->userName, login->password);

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

    int Send(DataHeader *dh, int cs)
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
