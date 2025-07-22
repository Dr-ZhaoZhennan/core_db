#pragma once
#include "SQLStatement.h"
#include <string>
#include <vector>

class SelectStatement : public SQLStatement {
public:
    SelectStatement(const std::string& tableName, const std::vector<std::string>& columns)
        : tableName(tableName), columns(columns) {}
    std::string getType() const override { return "SELECT"; }
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<std::pair<std::string, std::string>> whereConds; // 多条件
    std::string orderByCol;
    bool orderDesc = false;
    int limit = -1;
}; 