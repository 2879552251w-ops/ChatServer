#pragma once

#include <string>
using UserId=long long;
class User      //其实是用户状态信息，所以并不是对象语义，而是值语义
{
public:
    User(UserId id = -1,
         std::string name = "",
         std::string pwd = "",
         std::string state = "offline")
        : id_(id),
          name_(name),
          pwd_(pwd),
          state_(state)
    {
    }

    void setId(UserId id) { id_ = id; }
    void setName(std::string name) { name_ = name; }
    void setPwd(std::string pwd) { pwd_ = pwd; }
    void setState(std::string state) { state_ = state; }

    UserId getId() { return id_; }
    std::string getName() { return name_; }
    std::string getPwd() { return pwd_; }
    std::string getState() { return state_; }

private:
    UserId id_;
    std::string name_;
    std::string pwd_;
    std::string state_;
};