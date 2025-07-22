#pragma once
#include "SQLStatement.h"
#include <string>
#include <vector>

class InsertStatement : public SQLStatement {
public:
    InsertStatement(const std::string& tableName, const std::vector<std::string>& values)
        : tableName(tableName), values(values) {}
    std::string getType() const override { return "INSERT"; }
    std::string tableName;
    std::vector<std::string> values;
}; 