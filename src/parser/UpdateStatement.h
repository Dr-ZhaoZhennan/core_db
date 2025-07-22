#pragma once
#include "SQLStatement.h"
#include <string>
#include <vector>

class UpdateStatement : public SQLStatement {
public:
    UpdateStatement(const std::string& tableName, const std::vector<std::string>& columns, const std::vector<std::string>& values)
        : tableName(tableName), columns(columns), values(values) {}
    std::string getType() const override { return "UPDATE"; }
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<std::string> values;
    std::string whereCol;
    std::string whereVal;
}; 