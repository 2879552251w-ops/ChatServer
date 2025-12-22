#pragma once

enum class EnMsgType
{
    LOGIN_MSG = 1,      //登录
    LOGIN_MSG_ACK=2,    //登录回复
    REG_MSG = 3,        //注册
    REG_MSG_ACK=4,      //注册回复
    ONE_CHAT_MSG=5,     //点对点聊天
    ADD_FRIEND_MSG=6,   //添加好友

    CREATE_GROUP_MSG=7, //建群
    ADD_GROUP_MSG=8,    //群加成员
    GROUP_CHAT_MSG=9,   //群聊天

    LOGOUT_MSG=10       //注销
};