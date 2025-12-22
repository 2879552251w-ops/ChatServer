#include "OfflineMessageModel.hpp"
#include "db.hpp"

// 插入离线消息
void OfflineMsgModel::insert(UserId id, std::string msg)
{
    char sql[1024]={0};
    sprintf(sql,"insert into offlinemessage values('%lld','%s')",
    id,msg.data());
    MySQL sq;
    if(sq.connect())
    {
        sq.update(sql);
    }
}
// 删除离线消息
void OfflineMsgModel::erase(UserId id)
{
    char sql[1024]={0};
    sprintf(sql,"delete from offlinemessage where userid=%lld",id);
    MySQL sq;
    if(sq.connect())
    {
        sq.update(sql);
    }
}
// 查询离线消息
std::vector<std::string> OfflineMsgModel::query(UserId id)
{
    char sql[1024]={0};
    sprintf(sql,"select message from offlinemessage  where userid=%lld",id);
    MySQL sq;
    std::vector<std::string> msg;
    if(sq.connect())
    {
        MYSQL_RES *res = sq.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row; 
            while(row= mysql_fetch_row(res))
            {
                msg.push_back(row[0]);
            }
        }
        mysql_free_result(res);
    }
    return msg;
}