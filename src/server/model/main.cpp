#include "ChatServer.hpp"
#include "ChatService.hpp"
#include "signal.h"

void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main()
{
    signal(SIGINT,resetHandler);
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1",5005);
    
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();

    return 0;
}