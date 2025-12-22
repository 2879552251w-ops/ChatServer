#include "ChatServer.hpp"
#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include <json.hpp>
#include "ChatService.hpp"
#include "public.hpp"

using json = nlohmann::json;
ChatServer::ChatServer(muduo::net::EventLoop *loop,
                       const muduo::net::InetAddress &listenAddr,
                       const std::string &nameArg)
    : server_(loop,
              listenAddr,
              nameArg,muduo::net::TcpServer::kReusePort),
      loop_(loop)
{
    server_.setConnectionCallback([this](const muduo::net::TcpConnectionPtr &conn)
                                  {
                                      if (conn->connected())
                                      {
                                          LOG_INFO<< conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << " Online";
                                      }
                                      else
                                      {
                                          conn->shutdown();
                                          ChatService::instance()->clientCloseException(conn);
                                          LOG_ERROR<< conn->peerAddress().toIpPort() << " unconnected";
                                      } });

    server_.setMessageCallback(
        [this](const muduo::net::TcpConnectionPtr &conn,
               muduo::net::Buffer *buffer,
               muduo::Timestamp time)
        {
            std::string buf = buffer->retrieveAllAsString();
            // 数据反序列化
            json js;
            try
            {
                js = nlohmann::json::parse(buf);
                // 正常处理
            }
            catch (const nlohmann::json::parse_error &e)
            {
                LOG_ERROR << "json parse error: " << e.what();
                return; // 丢弃这次消息
            }

            // 解耦网络模块和业务模块
            // 通过js["msgid"] 获取=》 业务handler=》conn json time
            if (!js.contains("msgid") || !js["msgid"].is_number_integer())
            {
                // 给客户端返回错误信息也行，这里直接打印示意
                LOG_ERROR << "msgid 字段不存在或不是整数";
                // conn->send("not key or not int");
                return;
            }
            //dispatch
            int msgIdInt = js["msgid"].get<int>();
            EnMsgType msgType = static_cast<EnMsgType>(msgIdInt);
            auto msgHandler = ChatService::instance()->getServe(msgType);
            msgHandler(conn, js, time);
        });
    //暂时这样，集群肯定要改
    ChatService::instance()->reset();
    server_.setThreadNum(4);
}
