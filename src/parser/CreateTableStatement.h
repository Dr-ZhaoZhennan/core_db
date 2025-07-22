#pragma once
#include "SQLStatement.h"
#include <string>
#include <vector>

class CreateTableStatement : public SQLStatement {
public:
    CreateTableStatement(const std::string& tableName, const std::vector<std::string>& columns)
        : tableName(tableName), columns(columns) {}
    std::string getType() const override { return "CREATE_TABLE"; }
    std::string tableName;
    std::vector<std::string> columns;
}; 