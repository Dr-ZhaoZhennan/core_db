#pragma once
#include <string>
#include <map>
#include <vector>

class CatalogManager {
public:
    static CatalogManager& getInstance();
    bool createTable(const std::string& name, const std::vector<std::string>& columns);
    std::vector<std::string> getTableColumns(const std::string& name) const;
    bool dropTable(const std::string& name);
    std::vector<std::string> listTables() const;
private:
    CatalogManager() = default;
    std::map<std::string, std::vector<std::string>> tableColumns;
}; 