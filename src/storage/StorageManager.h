#pragma once
#include <string>
#include <map>
#include <memory>
#include "Table.h"

class StorageManager {
public:
    static StorageManager& getInstance();
    bool createTable(const std::string& name);
    std::shared_ptr<Table> getTable(const std::string& name);
    bool dropTable(const std::string& name);
    std::vector<std::string> listTables() const;
private:
    StorageManager() = default;
    std::map<std::string, std::shared_ptr<Table>> tables;
}; 