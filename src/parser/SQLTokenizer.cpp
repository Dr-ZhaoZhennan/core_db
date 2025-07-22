#include "SQLTokenizer.h"
#include <vector>
#include <string>
#include <cctype>

std::vector<std::string> SQLTokenizer::tokenize(const std::string& sql) {
    std::vector<std::string> tokens;
    std::string token;
    bool inString = false;
    for (size_t i = 0; i < sql.size(); ++i) {
        char c = sql[i];
        if (c == '\'') {
            inString = !inString;
            token += c;
        } else if (inString) {
            token += c;
        } else if (std::isspace(c)) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else if (c == '(' || c == ')' || c == ',' || c == ';') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            tokens.push_back(std::string(1, c));
        } else {
            token += c;
        }
    }
    if (!token.empty()) tokens.push_back(token);
    return tokens;
} 