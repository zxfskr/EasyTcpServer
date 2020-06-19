#include "EasyTcpServer.hpp"

using namespace std;

void sighandler(int sig)
{
    std::cout << "receive signal"
              << sig
              << std::endl;
}

vector<int> g_sockets;

int main(int argc, char **argv)
{
    // signal(SIGINT, sighandler);
    // signal(SIGQUIT, sighandler);
    // signal(SIGHUP, sighandler);
    // 防止客户端断开连接时server程序死亡
    signal(SIGPIPE, sighandler);

    EasyTcpServer server;
    server.InitSock();
    server.Bind(NULL, 10086);
    server.Listen();

    while (true)
    {
        /* code */
        if (-1 == server.OnRun())
        {
            break;
        }
    }
    server.Close();
    return 0;
}
