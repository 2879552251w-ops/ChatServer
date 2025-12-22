#include "UserModel.hpp"
#include "db.hpp"


bool UserModel::insert(User& user)
{
    char sql[1024]={0};
    sprintf(sql,"insert into user(name,password,state) values('%s','%s','%s')",
    user.getName().data(),user.getPwd().data(),user.getState().data());
    MySQL sq;
    if(sq.connect())
    {
        if(sq.update(sql))
        {
            user.setId(sq.getid());
            return true;
        }
    }
    return false;
}

User UserModel::query(UserId id)
{
    char sql[1024]={0};
    sprintf(sql,"select * from user where id=%lld",id);

    MySQL mysql;
    if(mysql.connect()) 
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr)
            {
                User user;
                user.setId(atoll(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
            mysql_free_result(res);
        }
    }   
    return User();
}

bool UserModel::update(User &user)
{
    char sql[1024]={0};
    sprintf(sql,"update user set state='%s' where id=%lld",user.getState().data(),user.getId());
    MySQL sq;
    if(sq.connect())
    {
        if(sq.update(sql))
        {
            user.setId(sq.getid());
            return true;
        }
    }
    return false;
}


void UserModel::resetState()
{
    char sql[1024]="update user set state='offline'";
    MySQL sq;
    if(sq.connect())
    {
        sq.update(sql);
    }
}