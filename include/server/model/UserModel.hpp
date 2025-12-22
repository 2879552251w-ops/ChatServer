#pragma once
#include "User.hpp"

class UserModel //strate class 封装数据库底层操作
{
public:
    bool insert(User &user);
    User query(UserId id);
    bool update(User &user);
    void resetState();
};