#pragma once
#include <string>
#include <vector>

class SQLTokenizer {
public:
    static std::vector<std::string> tokenize(const std::string& sql);
}; 