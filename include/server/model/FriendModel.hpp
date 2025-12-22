#pragma once
#include <vector>
#include "User.hpp"
class FriendModel
{
    using UserId = long long;

public:
    // 添加好友
    void add(UserId id, UserId friendid);
    // 返回用户列表
    std::vector<User> query(UserId id);
};