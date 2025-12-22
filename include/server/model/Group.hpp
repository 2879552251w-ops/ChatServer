#pragma once
#include <string>
#include <vector>
#include "GroupUser.hpp"
using GroupId=long long;
class Group
{
public:
Group(GroupId id = -1,
         std::string name = "",
         std::string  desc = "")
        : id_(id),
          name_(name),
          desc_(desc)
    {
    }

    bool operator <(const Group& g) const {return id_<g.id_;}
    void setId(GroupId id)  { id_ = id; }
    void setName(std::string name) { name_ = name; }
    void setDesc(std::string desc) { desc_ = desc; }

    GroupId getId() const { return id_; }
    std::string getName() const { return name_; }
    std::string getDesc() const { return desc_; }
    std::vector<GroupUser>& getUsers()  {return users_;}
private:
    GroupId id_;
    std::string name_;
    std::string desc_;
    std::vector<GroupUser> users_;
};