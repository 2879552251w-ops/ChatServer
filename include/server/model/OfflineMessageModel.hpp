#pragma once
#include <string>
#include <vector>
class OfflineMsgModel//方法类，封装离线消息储存的数据库业务
{
    using UserId=long long;
public:
    //插入离线消息
    void insert(UserId id,std::string msg);
    //删除离线消息
    void erase(UserId id);
    //查询离线消息
    std::vector<std::string> query(UserId id);
};