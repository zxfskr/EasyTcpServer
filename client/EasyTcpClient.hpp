#ifndef __EasyTcpClient_hpp__
#define __EasyTcpClient_hpp__

#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <strings.h>

#include "MessageHeader.hpp"

class EasyTcpClient
{
private:
    /* data */
    int _sock;

public:
    EasyTcpClient(/* args */)
    {
        _sock = -1;
    }

    virtual ~EasyTcpClient()
    {
        if (-1 != _sock)
            Close();
    }
    //初始化socket
    int InitSocket()
    {
        int ret = -1;
        _sock = socket(PF_INET, SOCK_STREAM, 0);
        if (_sock == -1)
        {
            printf("failed to create socket!\n");
            return -1;
        }

        int on = 1;
        ret = setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (ret == -1)
        {
            printf("failed to set socket options!");
            Close();
            return -1;
        }
        return _sock;
    }

    //链接服务器
    int Connect(char *ip, unsigned short port)
    {
        if (-1 == _sock)
        {
            printf("未初始化socket!\n");
            return -1;
        }
        int ret = -1;
        struct sockaddr_in remoteaddr;
        remoteaddr.sin_family = AF_INET;
        remoteaddr.sin_port = htons(port);
        remoteaddr.sin_addr.s_addr = inet_addr(ip);
        bzero(&(remoteaddr.sin_zero), 8);

        ret = connect(_sock, (struct sockaddr *)&remoteaddr, sizeof(struct sockaddr));
        if (ret < 0)
        {
            printf("failed to connect server!\n");
            return -1;
        }
        return 0;
    }

    void Close()
    {
        close(_sock);
        _sock = -1;
    }
    int count = 0;
    //处理消息
    bool OnRun()
    {
        if (IsRun())
        {
            int ret = -1;
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(_sock, &rfds);

            timeval t = {1, 0};
            ret = select(_sock + 1, &rfds, NULL, NULL, &t);
            // printf("select ret %d count %d\n", ret, count++);
            if (ret < 0)
            {
                printf("<socket=%d>select error\n", _sock);
                return false;
            }
            else if (0 == ret)
            {
                // printf("空闲时间\n");
            }
            else
            {
                if (FD_ISSET(_sock, &rfds))
                {
                    if (-1 == RecvData())
                    {
                        printf("<socket=%d>链接断开\n", _sock);
                        return false;
                    }
                }
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool IsRun()
    {
        return -1 != _sock;
    }

    #define RECV_BUFF_SIZE 10240
    char _szRecv[RECV_BUFF_SIZE] = {};
    char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
    int _lastPos = 0;
    //接收数据, 要处理粘包拆分包
    int RecvData()
    {
        int nlen = recv(_sock, (char *)&_szRecv, RECV_BUFF_SIZE, 0);
        if (nlen == 0)
        {
            Close();

            bzero(_szMsgBuf, RECV_BUFF_SIZE * 10);
            _lastPos = 0;
            printf("链接断开\n");
            return -1;
        }

         //将
        memcpy(_szMsgBuf + _lastPos, _szRecv, nlen);
        //消息缓冲区
        _lastPos += nlen;
        printf("接受数据的长度;%d data_header len: %d\n", _lastPos, sizeof(DataHeader));
        while (_lastPos >= sizeof(DataHeader))
        {
            struct DataHeader *dh = (DataHeader *)_szMsgBuf;
            printf("dh->datalength : %d\n", dh->dataLength);
            if (_lastPos >= dh->dataLength)
            {
                OnNetMsg(dh);
                // 记录处理消息长度
                int nSize = dh->dataLength;
                memcpy(_szMsgBuf, _szMsgBuf + nSize, _lastPos - nSize);
                //滑动窗口
                _lastPos = _lastPos - nSize;
            }else
            {
                //剩余数据长度不够一条完整的消息
                break;

            }
            
        }

        return 0;
    }

    void OnNetMsg(DataHeader *dh)
    {

        switch (dh->cmd)
        {
        case CMD_NEW_USER_JOIN:
        {
            NewUserJoin *nuj = (NewUserJoin *)dh;
            printf("新用户加入， login socket num :%d\n", nuj->socket);
        }
        break;
        case CMD_LOGIN_RESULT:
        {
            LoginResult *lir = (LoginResult *)dh;
            printf("登录结果:%d,数据长度 :%d\n", lir->result, lir->dataLength);
        }
        break;
        default:
        {
            // recv(socket_fd, (char *)&buf, 1024, 0);
            printf("错误的命令\n");
            Error err;
            // send(socket_fd, (const void *)&err, sizeof(err), 0);
        }
        break;
        }
    }

    int SendData(DataHeader *dh)
    {
        if (IsRun() && dh != NULL)
        {
            return send(_sock, (const void *)dh, dh->dataLength, 0);
        }
        return -1;
    }
};

#endif // !__EA