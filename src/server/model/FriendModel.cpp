#include "FriendModel.hpp"
#include "db.hpp"
// 添加好友
void FriendModel::add(UserId id, UserId friendid)
{
    char sql[1024]={0};
    sprintf(sql,"insert into friend values('%lld','%lld')",id,friendid);
    MySQL sq;
    if(sq.connect())
    {
        sq.update(sql);
    }
}
// 返回用户列表
std::vector<User> FriendModel::query(UserId id)
{
    char sql[1024]={0};
    sprintf(sql,"select b.friendid,a.name,a.state from user a join friend b on a.id=b.friendid where b.userid=%lld",id);
    MySQL sq;
    std::vector<User> msg;
    if(sq.connect())
    {
        MYSQL_RES *res = sq.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row; 
            while(row= mysql_fetch_row(res))
            {
                msg.emplace_back(atoll(row[0]),row[1],"",row[2]);
            }
        }
        mysql_free_result(res);
    }
    return msg;
}