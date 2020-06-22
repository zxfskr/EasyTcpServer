#include <thread>

#include "EasyTcpClient.hpp"
#include <signal.h>

#define PORT 10086
#define MESSAGE_SIZE 1024

using namespace std;

void test_cout(const char *msg)
{
    std::cout << msg << std::endl;
}

bool g_bRun = true;
void cmdThread(EasyTcpClient *client)
{
    char cmdBuf[256] = {};
    while (g_bRun)
    {
        /* code */
        // char cmdBuf[256] = {};
        // scanf("%s", cmdBuf);
        strcpy(cmdBuf, "login");
        // usleep(10);

        if (0 == strcmp(cmdBuf, "exit"))
        {
            printf("退出cmdThread 线程\n");
            g_bRun = false;
            return;
        }
        else if (0 == strcmp(cmdBuf, "login"))
        {
            Login lgi;
            strcpy(lgi.userName, "haha");
            strcpy(lgi.password, "heihei");
            client->SendData(&lgi);
        }
        else if (0 == strcmp(cmdBuf, "logout"))
        {
            Logout lgo;
            client->SendData(&lgo);
        }
        else
        {
            printf("不支持的命令\n");
        }
    }
}

const int cCount = 100;
EasyTcpClient *clients[cCount];

void sighandler(int sig)
{
    std::cout << "receive signal"
              << sig
              << std::endl;

}   

int main(int argc, char **argv)
{
    // const int cCount = 10;

    // signal(SIGINT, sighandler);

    // EasyTcpClient *clients[cCount];
    int i = 0;
    for (i = 0; i < cCount; i++)
    {
        clients[i] = new EasyTcpClient();
        clients[i]->InitSocket();
        clients[i]->Connect((char*)"127.0.0.1", 10086);

        printf("客户端链接 %d\n", i);
    }
    // thread t1(cmdThread, &client);
    // t1.detach();

    Login lgi;
    strcpy(lgi.userName, "haha");
    strcpy(lgi.password, "heihei");

    while (g_bRun)
    {

        for (i=0; i<cCount; i++){
            clients[i]->SendData(&lgi);
            clients[i]->OnRun();
        }
        // if (!client.OnRun())
        // {
        //     g_bRun = false;
        //     break;
        // }
    }

    // t1.detach();
    // t1.join();
fail:
    g_bRun = false;
    // t1.close()
    // t1.join();

    for (i = 0; i < cCount; i++)
        clients[i]->Close();
    printf("已退出\n");
    return 0;
}
