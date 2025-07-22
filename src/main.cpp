#include <iostream>
#include "parser/SQLParser.h"
#include "executor/Executor.h"

int main() {
    std::cout << "欢迎使用 MyDB 数据库内核！\n";
    std::cout << "请输入SQL语句，输入exit退出。\n";
    std::string sql;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, sql);
        if (sql == "exit") break;
        // 解析SQL
        auto stmt = SQLParser::parse(sql);
        if (!stmt) {
            std::cout << "SQL解析错误！\n";
            continue;
        }
        // 执行SQL
        Executor::execute(stmt);
    }
    std::cout << "再见！\n";
    return 0;
} 