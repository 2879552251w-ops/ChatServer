#pragma once
#include <string>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
class EventLoop;
class InetAddress;

class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop* loop,
            const muduo::net::InetAddress& listenAddr,
            const std::string& nameArg);
    void start()
    {
        server_.start();
    }
private:
    muduo::net::TcpServer server_;
    muduo::net::EventLoop *loop_;
};
