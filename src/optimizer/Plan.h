#pragma once
#include <memory>
#include "../parser/SQLStatement.h"

class Plan {
public:
    Plan(const std::shared_ptr<SQLStatement>& stmt) : stmt(stmt) {}
    std::shared_ptr<SQLStatement> getStatement() const { return stmt; }
private:
    std::shared_ptr<SQLStatement> stmt;
}; 