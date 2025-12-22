#pragma once
#include "Group.hpp"
#include <vector>
#include "db.hpp"
class GroupModel
{
public:
    bool createGroup(Group &group);
    void addGroupUser(UserId userid,GroupId groupid,std::string role);
    std::vector<Group> queryGroup(UserId userid);
    //主要用于群聊，拿到组里所有用户id，转发
    std::vector<UserId> queryGroupUsers(UserId userid,GroupId groupid);
};