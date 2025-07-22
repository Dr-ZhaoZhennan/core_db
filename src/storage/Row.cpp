#include "Row.h"

Row::Row(const std::vector<std::string>& values) : values(values) {}

std::vector<std::string> Row::getValues() const {
    return values;
} 