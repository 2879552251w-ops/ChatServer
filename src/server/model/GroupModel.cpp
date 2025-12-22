#include "GroupModel.hpp"

bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname,groupdesc) values('%s','%s')",
            group.getName().data(), group.getDesc().data());
    MySQL sq;
    if (sq.connect())
    {
        if (sq.update(sql))
        {
            group.setId(sq.getid());
            return true;
        }
    }
    return false;
}
void GroupModel::addGroupUser(UserId userid, GroupId groupid, std::string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values('%lld','%lld','%s')", groupid, userid, role.data());
    MySQL sq;
    if (sq.connect())
    {
        sq.update(sql);
    }
}
std::vector<Group> GroupModel::queryGroup(UserId userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a join \
    groupuser b on a.id=b.groupid where b.userid=%lld",
            userid);
    MySQL sq;
    std::vector<Group> msg;
    if (sq.connect())
    {
        MYSQL_RES *res = sq.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while (row = mysql_fetch_row(res))
            {
                msg.emplace_back(atoll(row[0]), row[1], row[2]);
            }
        }
        mysql_free_result(res);
    }
    for (Group &g : msg)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a join \
    groupuser b on a.id=b.userid where b.groupid=%lld",
                g.getId());

        MYSQL_RES *res = sq.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while (row = mysql_fetch_row(res))
            {
                g.getUsers().emplace_back(atoll(row[0]), row[1], "", row[2], row[3]);
            }
        }
        mysql_free_result(res);
    }
    return msg;
}
// 主要用于群聊，拿到组里所有用户id，转发
std::vector<UserId> GroupModel::queryGroupUsers(UserId userid, GroupId groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid=%lld and userid!=%lld", groupid, userid);
    MySQL sq;
    std::vector<UserId> msg;
    if (sq.connect())
    {
        MYSQL_RES *res = sq.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while (row = mysql_fetch_row(res))
            {
                msg.emplace_back(atoll(row[0]));
            }
        }
        mysql_free_result(res);
    }
    return msg;
}