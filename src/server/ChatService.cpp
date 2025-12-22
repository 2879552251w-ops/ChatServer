#include "ChatService.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include "UserModel.hpp"
ChatService *ChatService::instance()
{
    static ChatService instance;
    return &instance;
}

ChatService::ChatService()
{
    msgHandMap_[EnMsgType::LOGIN_MSG] = std::bind(&ChatService::login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    msgHandMap_.emplace(EnMsgType::REG_MSG, std::bind(&ChatService::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    msgHandMap_.emplace(EnMsgType::ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    msgHandMap_.emplace(EnMsgType::ADD_FRIEND_MSG, std::bind(&ChatService::addfriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    msgHandMap_.emplace(EnMsgType::CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    msgHandMap_.emplace(EnMsgType::ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    msgHandMap_.emplace(EnMsgType::GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    msgHandMap_.emplace(EnMsgType::LOGOUT_MSG, std::bind(&ChatService::logout, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    if (redis_.connect())
    {
        redis_.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this, std::placeholders::_1, std::placeholders::_2));
    }
}

void ChatService::reset()
{
    usermodel_.resetState();
}

MsgHandler ChatService::getServe(EnMsgType en)
{
    auto it = msgHandMap_.find(en);
    if (it == msgHandMap_.end())
    {
        return [=](const muduo::net::TcpConnectionPtr &conn,
                   json &js,
                   muduo::Timestamp time)
        {
            LOG_ERROR << "msgid:" << static_cast<int>(en) << "can not find handler";
        };
    }
    return it->second;
}
// 处理登录业务
void ChatService::login(const muduo::net::TcpConnectionPtr &conn,
                        json &js,
                        muduo::Timestamp time)
{
    UserId id = js["id"];
    std::string pwd = js["password"];
    User user = usermodel_.query(id);
    if (user.getId() != -1 && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 用户已登录，
            //  登录失败
            json response;
            response["msgid"] = static_cast<int>(EnMsgType::LOGIN_MSG_ACK);
            response["errno"] = 2;
            response["errmsg"] = "重复登录";
            conn->send(response.dump());
        }
        else
        {
            // 记录登录用户
            {
                std::lock_guard<std::mutex> lock(mutex_);
                users_.emplace(id, conn);
            }
            // id用户登录成功以后，像redis订阅channel（id）
            redis_.subscribe(id);

            // 登录成功
            user.setState("online");
            usermodel_.update(user);
            json response;
            response["msgid"] = static_cast<int>(EnMsgType::LOGIN_MSG_ACK);
            response["errno"] = 0;
            response["id"] = id;
            response["name"] = user.getName();
            // 查询是否有离线消息
            std::vector<std::string> vec = offlinemsgmodel_.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                offlinemsgmodel_.erase(id);
            }
            // 查询该用户的好友信息
            std::vector<User> friends = friendmodel_.query(id);
            if (!friends.empty())
            {
                std::vector<std::string> msg;
                for (auto &user : friends)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    msg.emplace_back(js.dump());
                }
                response["friends"] = msg;
            }

            std::vector<Group> groups = groupmodel_.queryGroup(id);
            if (!groups.empty())
            {
                std::vector<json> groupmsg;
                for (auto &g : groups)
                {
                    json js;
                    js["id"] = g.getId();
                    js["groupname"] = g.getName();
                    js["groupdesc"] = g.getDesc();
                    std::vector<json> userV;
                    for (auto &user : g.getUsers())
                    {
                        json js2;
                        js2["id"] = user.getId();
                        js2["name"] = user.getName();
                        js2["state"] = user.getState();
                        js2["role"] = user.getRole();
                        userV.emplace_back(js2);
                    }
                    js["users"] = userV;
                    groupmsg.emplace_back(js);
                }
                response["groups"] = groupmsg;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败
        json response;
        response["msgid"] = static_cast<int>(EnMsgType::LOGIN_MSG_ACK);
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}
// 处理注册业务
void ChatService::reg(const muduo::net::TcpConnectionPtr &conn,
                      json &js,
                      muduo::Timestamp time)
{
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    if (usermodel_.insert(user))
    {
        // 注册成功
        json respose;
        respose["msgid"] = static_cast<int>(EnMsgType::REG_MSG_ACK);
        respose["errno"] = 0;
        respose["id"] = user.getId();
        conn->send(respose.dump());
    }
    else
    {
        // 注册失败
        json respose;
        respose["msgid"] = static_cast<int>(EnMsgType::REG_MSG_ACK);
        respose["errno"] = 1;
        respose["id"] = user.getId();
        conn->send(respose.dump());
    }
}

void ChatService::oneChat(const muduo::net::TcpConnectionPtr &conn,
                          json &js,
                          muduo::Timestamp time)
{
    UserId peerid = js["friendid"].is_number_integer() ? js["friendid"].get<int>() : 0;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = users_.find(peerid);
        if (it != users_.end())
        {
            // 对方在线，直接发
            it->second->send(js.dump());
            return;
        }
    }
    // 是否在别的主机上线
    User user = usermodel_.query(peerid);
    if (user.getState() == "online")
    {
        redis_.publish(peerid, js.dump());
    }
    else // 离线消息
        offlinemsgmodel_.insert(peerid, js.dump());
}
void ChatService::logout(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    UserId id = js["id"];
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = users_.find(id);
        if (it != users_.end())
        {
            users_.erase(it);
        }
    }

    // 用户注销，取消关注信息
    redis_.unsubscribe(id);
    // 构造默认就是offline
    User user(id);
    // user.setId(id);
    // user.setState("offline");
    usermodel_.update(user);
}

void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr &conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = users_.begin();
        for (; it != users_.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                users_.erase(it);
                break;
            }
        }
    }

    // 用户注销，取消关注信息
    redis_.unsubscribe(user.getId());

    if (user.getId() != -1)
    {
        user.setState("offline");
        usermodel_.update(user);
    }
}

// 添加好友
void ChatService::addfriend(const muduo::net::TcpConnectionPtr &conn,
                            json &js,
                            muduo::Timestamp time)
{
    UserId id = js["id"].get<UserId>();
    UserId peerid = js["friendid"].get<UserId>();

    friendmodel_.add(id, peerid);
}

// 创建群组
void ChatService::createGroup(const muduo::net::TcpConnectionPtr &conn,
                              json &js,
                              muduo::Timestamp time)
{
    UserId userid = js["id"].get<UserId>();
    std::string groupname = js["groupname"];
    std::string desc = js["groupdesc"];
    Group group(-1, groupname, desc);
    if (groupmodel_.createGroup(group))
    {
        groupmodel_.addGroupUser(userid, group.getId(), "creator");
    }
}
// 加入群组
void ChatService::addGroup(const muduo::net::TcpConnectionPtr &conn,
                           json &js,
                           muduo::Timestamp time)
{
    UserId userid = js["id"].get<UserId>();
    GroupId groupid = js["groupid"].get<GroupId>();
    groupmodel_.addGroupUser(userid, groupid, "normal");
}
// 群聊天
void ChatService::groupChat(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    UserId userid = js["id"].is_number() ? js["id"].get<UserId>() : 0;
    GroupId groupid = js["groupid"].is_number() ? js["groupid"].get<GroupId>() : 0;
    // 0. 提前序列化 (极大提升 CPU 效率)
    std::string msg_str = js.dump();

    std::vector<UserId> users = groupmodel_.queryGroupUsers(userid, groupid);
    std::vector<muduo::net::TcpConnectionPtr> online;
    std::vector<UserId> offline;

    // 预分配内存，避免 vector 扩容带来的开销
    online.reserve(users.size());
    offline.reserve(users.size());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (UserId id : users)
        {
            auto it = users_.find(id);
            if (it != users_.end())
            {
                // 这里拷贝了 shared_ptr，引用计数+1
                // 只要 online 容器不销毁，Connection 对象就一定活着
                online.push_back(it->second);
            }
            else
            {
                offline.push_back(id);
            }
        }
    } // 锁在这里释放

    // 1. 在线发送 (复用 serialized string)
    for (const auto &conn : online)
    {
        // Muduo 的 send 是线程安全的，可以直接调用
        // 且 muduo 内部会检查连接状态，但如果想更严谨可以加一句 if(conn->connected())
        conn->send(msg_str);
    }

    // 2. 离线处理
    // 建议优化：如果在 model 层能实现 batchInsert(offline, msg_str) 会比循环 insert 快得多
    for (UserId id : offline)
    {
        User user = usermodel_.query(id);
        if (user.getState() == "online")
        {
            redis_.publish(id, js.dump());
        }
        offlinemsgmodel_.insert(id, msg_str);
    }
}

// 从redis消息队列获得消息
void ChatService::handleRedisSubscribeMessage(UserId userid, std::string msg)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = users_.find(userid);
    if(it!=users_.end())
    {
        it->second->send(msg);
        return;
    }

    offlinemsgmodel_.insert(userid,msg);
}