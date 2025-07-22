#pragma once
#include "SQLStatement.h"
#include <string>

class DeleteStatement : public SQLStatement {
public:
    DeleteStatement(const std::string& tableName) : tableName(tableName) {}
    std::string getType() const override { return "DELETE"; }
    std::string tableName;
    std::string whereCol;
    std::string whereVal;
}; 