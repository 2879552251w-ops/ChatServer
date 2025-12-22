#pragma once
#include <mutex>
#include <functional>
#include <memory>
#include <muduo/base/Timestamp.h>
#include <muduo/net/TcpConnection.h>
#include "json.hpp"
#include <unordered_map>
#include "public.hpp"
#include "UserModel.hpp"
#include "OfflineMessageModel.hpp"
#include "FriendModel.hpp"
#include "GroupModel.hpp"
#include "redis.hpp"
using json = nlohmann::json;
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr &conn,
                                      json &js,
                                      muduo::Timestamp time)>;

class ChatService
{
public:
    static ChatService *instance();
    // 处理登录业务
    void login(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);
    //注销
    void logout(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);
    // 处理注册业务
    void reg(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);
    //一对一聊天业务
    void oneChat(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);
    //添加好友
    void addfriend(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);
    //创建群组
    void createGroup(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);
    //加入群组
    void addGroup(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);
    //群聊天
    void groupChat(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);
    //客户端异常关闭
    void clientCloseException(const muduo::net::TcpConnectionPtr &conn);
    //从redis消息队列获得消息
    void handleRedisSubscribeMessage(UserId userid,std::string msg);
    //重置
    void reset();
    //获取业务类型
    MsgHandler getServe(EnMsgType);

private:
    ChatService();
    // 对任意可显示转换为size_t的类型都可作为键
    struct EnumClassHash
    {
        template <typename T>
        std::size_t operator()(T t) const noexcept
        {
            return static_cast<std::size_t>(t);
        }
    };

    std::unordered_map<EnMsgType, MsgHandler, EnumClassHash> msgHandMap_;//储存业务函数

    std::mutex mutex_;
    std::unordered_map<UserId,muduo::net::TcpConnectionPtr> users_;       //储存在线人员

    UserModel usermodel_;   //方法类，封装数据库代码
    OfflineMsgModel offlinemsgmodel_;
    FriendModel friendmodel_;
    GroupModel groupmodel_;
    Redis redis_;
};