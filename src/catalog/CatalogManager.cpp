#include "CatalogManager.h"

CatalogManager& CatalogManager::getInstance() {
    static CatalogManager instance;
    return instance;
}

bool CatalogManager::createTable(const std::string& name, const std::vector<std::string>& columns) {
    if (tableColumns.count(name)) return false;
    tableColumns[name] = columns;
    return true;
}

std::vector<std::string> CatalogManager::getTableColumns(const std::string& name) const {
    if (tableColumns.count(name)) return tableColumns.at(name);
    return {};
}

bool CatalogManager::dropTable(const std::string& name) {
    return tableColumns.erase(name) > 0;
}

std::vector<std::string> CatalogManager::listTables() const {
    std::vector<std::string> names;
    for (const auto& kv : tableColumns) names.push_back(kv.first);
    return names;
} 