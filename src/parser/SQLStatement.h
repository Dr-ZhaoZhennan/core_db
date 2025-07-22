#pragma once
#include <string>

class SQLStatement {
public:
    virtual ~SQLStatement() = default;
    virtual std::string getType() const = 0;
}; 