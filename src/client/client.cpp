#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include <thread>
#include <iostream>
#include "json.hpp"

#include "public.hpp"
#include "User.hpp"
#include "Group.hpp"
using json = nlohmann::json;

User G_Current_User;
std::vector<User> G_Current_Friends;
std::vector<Group> G_Current_GroupList;
bool G_islogging = false;

std::string gettime()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    tm tmnow;
    localtime_r(&now, &tmnow); // 把整数的时间转换成tm结构体。

    // 根据tm结构体拼接成中国人习惯的字符串格式。
    std::string stime = std::to_string(tmnow.tm_year + 1900) + "-" + std::to_string(tmnow.tm_mon + 1) + "-" + std::to_string(tmnow.tm_mday) + " " + std::to_string(tmnow.tm_hour) + ":" + std::to_string(tmnow.tm_min) + ":" + std::to_string(tmnow.tm_sec);
    return stime;
}

void showCurrentUserData()
{
    std::cout << "===========================login user==============================\n";
    std::cout << "current login user => id:" << G_Current_User.getId() << " name:" << G_Current_User.getName() << "\n";
    std::cout << "===========================friend list============================\n";
    if (!G_Current_Friends.empty())
    {
        for (auto &fr : G_Current_Friends)
        {
            std::cout << fr.getId() << " : " << fr.getName() << " " << fr.getState() << "\n";
        }
    }
    std::cout << "===========================group list==============================\n";
    if (!G_Current_GroupList.empty())
    {
        for (auto &gr : G_Current_GroupList)
        {
            std::cout << gr.getId() << " : " << gr.getName() << " " << gr.getDesc() << "\n";
            std::cout << "User List:\n";
            for (auto &guser : gr.getUsers())
            {
                std::cout << guser.getId() << " : " << guser.getName() << " " << guser.getState() << " role:" << guser.getRole() << "\n";
            }
        }
    }
    std::cout << "===========================offline msg===============================\n";
}

void readTaskHandler(int fd)
{
    while (true)
    {
        char buf[2048] = {0};
        int n = recv(fd, buf, 2048, 0);
        if (n <= 0)
        {
            std::cout << "用户已经注销" << std::endl;
            break;
        }

        json js = json::parse(buf);
        int msgtype = js["msgid"].is_number() ? js["msgid"].get<int>() : 0;
        if (msgtype == static_cast<int>(EnMsgType::ONE_CHAT_MSG))
        {
            std::cout << js["time"].get<std::string>() << " [" << js["id"] << "] " << js["name"].get<std::string>() << " said: " << js["msg"].get<std::string>() << std::endl;
            continue;
        }
        else if (msgtype == static_cast<int>(EnMsgType::GROUP_CHAT_MSG))
        {
            std::cout << "来自" << js["groupid"] << "群消息：" << js["time"].get<std::string>() << " [" << js["id"] << "] " << js["name"].get<std::string>() << " said: " << js["msg"].get<std::string>() << std::endl;
            continue;
        }
    }
}

void regist(int fd)
{
    char name[50] = {0};
    char password[50] = {0};
    std::cout << "UserName: ";
    std::cin.getline(name, 50);
    std::cout << "Password: ";
    std::cin.getline(password, 50);

    json js;
    js["msgid"] = static_cast<int>(EnMsgType::REG_MSG);
    js["name"] = name;
    js["password"] = password;
    std::string msg = js.dump();

    int n = send(fd, msg.data(), msg.length(), 0);
    if (n == -1)
    {
        perror("senderror");
    }
    else
    {
        char buffer[1024] = {0};
        n = recv(fd, buffer, 1024, 0);
        if (n == -1)
        {
            perror("recv");
        }
        else
        {
            json response = json::parse(buffer);
            if (response["errno"].get<int>() == 0)
            {
                std::cout << "注册成功,用户id为:" << response["id"] << ",请记好!" << std::endl;
            }
            else
            {
                std::cerr << "注册失败，用户名已存在!\n";
            }
        }
    }
    close(fd);
}

bool login(int fd)
{
    UserId id = 0;
    char password[50] = {0};
    char cid[10] = {0};
    std::cout << "UserId: ";
    // std::cin >> id;
    //  忽略这一行所有内容直至'\n'
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.getline(cid, 10);
    id = atol(cid);
    std::cout << "Password: ";
    std::cin.getline(password, 50);

    json js;
    js["msgid"] = static_cast<int>(EnMsgType::LOGIN_MSG);
    js["id"] = id;
    js["password"] = password;
    std::string msg = js.dump();

    int n = send(fd, msg.data(), msg.length(), 0);
    if (n == -1)
    {
        perror("senderror");
        exit(-1);
    }
    else
    {
        char buffer[1024] = {0};
        n = recv(fd, buffer, 1024, 0);
        if (n == -1)
        {
            perror("recv");
            exit(-1);
        }
        else
        {
            json response = json::parse(buffer);
            if (response["errno"].get<int>() == 0)
            {
                // 登陆成功
                G_Current_User.setId(id);
                G_Current_User.setName(response["name"]);
                G_islogging = true;
                std::cout << "登录,用户id为:" << response["id"] << ",请记好！\n";
                if (response.contains("friends"))
                {
                    std::vector<std::string> friends = response["friends"];
                    for (auto &s : friends)
                    {
                        json friends = json::parse(s);
                        G_Current_Friends.emplace_back(friends["id"].get<int>(),
                                                       friends["name"],
                                                       "",
                                                       friends["state"]);
                    }
                }
                if (response.contains("groups"))
                {
                    json groups = response["groups"];
                    for (auto &gjs : groups)
                    {
                        Group group;
                        group.setId(gjs["id"].get<UserId>());
                        group.setName(gjs["groupname"]);
                        group.setDesc(gjs["groupdesc"]);

                        json users = gjs["users"];
                        for (auto &user : users)
                        {
                            group.getUsers().emplace_back(user["id"].get<UserId>(),
                                                          user["name"].get<std::string>(),
                                                          "",
                                                          user["state"].get<std::string>(),
                                                          user["role"].get<std::string>());
                        }
                        G_Current_GroupList.emplace_back(group);
                    }
                }
                showCurrentUserData();
                if (response.contains("offlinemsg"))
                {
                    std::vector<std::string> offlinemsg = response["offlinemsg"];
                    for (auto &str : offlinemsg)
                    {

                        try
                        {
                            json js = json::parse(str);
                            int msgtype = js["msgid"].is_number() ? js["msgid"].get<int>() : 0;
                            if (msgtype == static_cast<int>(EnMsgType::ONE_CHAT_MSG))
                            {
                                std::cout << js["time"].get<std::string>() << " [" << js["id"] << "] " << js["name"] << " said: " << js["msg"] << "\n";
                                continue;
                            }
                            else if (msgtype == static_cast<int>(EnMsgType::GROUP_CHAT_MSG))
                            {
                                std::cout << "来自" << js["groupid"] << "群消息：" << js["time"].get<std::string>() << " [" << js["id"] << "] " << js["name"] << " said: " << js["msg"] << "\n";
                                continue;
                            }
                        }
                        catch (const nlohmann::json::parse_error &pe)
                        {
                            std::cout << "parse error:" << pe.what() << std::endl;
                        }
                    }
                    std::cout << "====================================================" << std::endl;
                }
                std::thread readTask(readTaskHandler, fd);
                readTask.detach();

                return true;
            }
            else
            {
                std::cerr << response["errmsg"] << "\n";
                close(fd);
                return false;
            }
        }
    }
    return false;
}

std::unordered_map<std::string, std::string> commandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:frientid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "添加群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"logout", "注销,格式logout"}};

void help(int, std::string)
{
    std::cout << "show command list >>>\n";
    for (auto &pair : commandMap)
    {
        std::cout << pair.first << " : " << pair.second << "\n";
    }
    std::cout << std::endl;
}

void addfriend(int fd, std::string id)
{
    UserId friendid = std::stol(id);
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::ADD_FRIEND_MSG);
    js["friendid"] = friendid;
    js["id"] = G_Current_User.getId();
    std::string buffer = js.dump();
    int n = send(fd, buffer.data(), buffer.length() + 1, 0);
    if (n == -1)
    {
        perror("add friend");
    }
}

void chat(int fd, std::string cmd)
{
    int idx = cmd.find(":");
    if (idx == -1)
    {
        std::cerr << "格式错误\n";
        return;
    }
    UserId friendid;
    try
    {
        friendid = std::stol(cmd.substr(0, idx));
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "格式错误\n";
        return;
    }
    std::string msg = cmd.substr(idx + 1, cmd.size() - idx);

    json js;
    js["msgid"] = static_cast<int>(EnMsgType::ONE_CHAT_MSG);
    js["name"] = G_Current_User.getName();
    js["id"] = G_Current_User.getId();
    js["friendid"] = friendid;
    js["time"] = gettime();
    js["msg"] = msg;
    std::string buffer = js.dump();
    int n = send(fd, buffer.data(), buffer.length() + 1, 0);
    if (n == -1)
    {
        perror("one to one chat");
    }
}

void creategroup(int fd, std::string cmd)
{
    int idx = cmd.find(":");
    if (idx == -1)
    {
        std::cerr << "格式错误\n";
    }
    std::string groupname = cmd.substr(0, idx);
    std::string groupdesc = cmd.substr(idx + 1, cmd.size() - idx);

    json js;
    js["msgid"] = static_cast<int>(EnMsgType::CREATE_GROUP_MSG);
    js["groupname"] = groupname;
    js["id"] = G_Current_User.getId();
    js["groupdesc"] = groupdesc;

    std::string buffer = js.dump();
    int n = send(fd, buffer.data(), buffer.length() + 1, 0);
    if (n == -1)
    {
        perror("create group");
    }
}
void addgroup(int fd, std::string cmd)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::ADD_GROUP_MSG);
    js["groupid"] = std::stol(cmd);
    js["id"] = G_Current_User.getId();

    std::string buffer = js.dump();
    int n = send(fd, buffer.data(), buffer.length() + 1, 0);
    if (n == -1)
    {
        perror("add group");
    }
    else
    {
        G_Current_GroupList.emplace_back(js["groupid"].get<GroupId>());
    }
}
void groupchat(int fd, std::string cmd)
{
    int idx = cmd.find(":");
    if (idx == -1)
    {
        std::cerr << "格式错误\n";
        return;
    }
    GroupId groupid;
    try
    {
        groupid = std::stol(cmd.substr(0, idx));
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "格式错误\n";
        return;
    }
    bool ismygroup = false;
    for (auto &g : G_Current_GroupList)
    {
        if (groupid == g.getId())
            ismygroup = true;
    }
    if (!ismygroup)
    {
        std::cout << "不存在该群" << std::endl;
        return;
    }
    std::string msg = cmd.substr(idx + 1);

    json js;
    js["msgid"] = static_cast<int>(EnMsgType::GROUP_CHAT_MSG);
    js["name"] = G_Current_User.getName();
    js["id"] = G_Current_User.getId();
    js["groupid"] = groupid;
    js["time"] = gettime();
    js["msg"] = msg;
    std::string buffer = js.dump();
    int n = send(fd, buffer.data(), buffer.length() + 1, 0);
    if (n == -1)
    {
        perror("group chat");
    }
}
void logout(int fd, std::string cmd)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::LOGOUT_MSG);
    js["id"] = G_Current_User.getId();

    std::string buffer = js.dump();
    int n = send(fd, buffer.data(), buffer.length() + 1, 0);
    if (n == -1)
    {
        perror("add group");
    }
    else
    {
        G_Current_Friends.clear();
        G_Current_GroupList.clear();
        G_islogging = false;
        close(fd);
    }
}

std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"logout", logout}};
void gotoChat(int fd)
{
    help(0, "");
    char buffer[1024] = {0};
    while (G_islogging)
    {
        std::cin.getline(buffer, 1024);
        std::string commandbuf(buffer);
        std::string command;
        int idx = commandbuf.find(":");
        if (idx == -1)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            std::cerr << "命令无效\n";
        }
        else
        {
            it->second(fd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
        }
    }
}
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("example: ChatClient 192.168.181.135 5005\n");
        return -1;
    }
    // 1. 关闭与 C 语言 stdio 的同步
    std::ios::sync_with_stdio(false);

    // 2. 解除 cin 和 cout 的绑定 (防止读之前强制刷新写缓冲区)
    // std::cin.tie(nullptr);

    while (true)
    {
        printf("==================================\n");
        printf("please choice:\n");
        printf("1.Regist\n");
        printf("2.Login\n");
        printf("3.Exit\n");
        printf("==================================\n");
        int choice;
        printf("Choice: ");
        scanf("%d", &choice);

        int clientfd = -1;
        if (choice == 1 || choice == 2)
        {
            clientfd = socket(AF_INET, SOCK_STREAM, 0);
            if (clientfd == -1)
            {
                perror("socket creation failed");
                continue; // 创建失败就重新循环
            }

            sockaddr_in server = {0};
            server.sin_family = AF_INET;
            server.sin_port = htons(atoi(argv[2]));
            server.sin_addr.s_addr = inet_addr(argv[1]);

            if (connect(clientfd, (sockaddr *)&server, sizeof(server)) == -1)
            {
                perror("connect failed");
                close(clientfd); // 失败记得关掉
                continue;
            }
        }

        switch (choice)
        {
        case 1:
            regist(clientfd);
            break;
        case 2:
            if (login(clientfd) == true)
                gotoChat(clientfd);
            break;
        case 3:
            exit(0);
        default:
            printf("have not this choice\n");
            break;
        }
    }
}