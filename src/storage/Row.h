#pragma once
#include <vector>
#include <string>

class Row {
public:
    Row(const std::vector<std::string>& values);
    std::vector<std::string> getValues() const;
private:
    std::vector<std::string> values;
}; 