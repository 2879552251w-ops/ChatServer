#pragma once 
#include "User.hpp"


class GroupUser: public User
{
public:
GroupUser(UserId id = -1,
         std::string name = "",
         std::string pwd = "",
         std::string state = "offline",
         std::string role = "normal")
        : User(id,name,pwd,state),
        role_(role)
    {
    }
    void setRole(std::string role) {this->role_ = role;}
    std::string getRole () {return role_;}

private:
    std::string role_;
};