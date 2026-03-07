#pragma once
#include <string>
#include <mysql/mysql.h>

// 数据库配置信息
namespace{
const std::string server = "127.0.0.1";
const std::string user = "root";
const std::string password = "229578";
const std::string dbname = "chat";
}
// 数据库操作类
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();
    // 释放数据库连接资源
    ~MySQL();
    // 连接数据库
    bool connect();
    // 更新操作
    bool update(std::string sql);
    // 查询操作
    MYSQL_RES *query(std::string sql);
    long long getid()
    {
        return mysql_insert_id(_conn);
    }
private:
    MYSQL *_conn;
};