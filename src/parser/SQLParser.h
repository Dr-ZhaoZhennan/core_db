#pragma once
#include <string>
#include <memory>
#include "SQLStatement.h"

class SQLParser {
public:
    static std::shared_ptr<SQLStatement> parse(const std::string& sql);
}; 