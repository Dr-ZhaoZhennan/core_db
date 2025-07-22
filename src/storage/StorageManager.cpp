#include "StorageManager.h"
#include "../catalog/CatalogManager.h"

StorageManager& StorageManager::getInstance() {
    static StorageManager instance;
    return instance;
}

bool StorageManager::createTable(const std::string& name) {
    if (tables.count(name)) return false;
    tables[name] = std::make_shared<Table>(name);
    // 设置字段信息
    auto cols = CatalogManager::getInstance().getTableColumns(name);
    tables[name]->setColumns(cols);
    tables[name]->saveToFile();
    return true;
}

std::shared_ptr<Table> StorageManager::getTable(const std::string& name) {
    if (tables.count(name)) return tables[name];
    // 尝试从文件加载
    auto t = std::make_shared<Table>(name);
    t->setColumns(CatalogManager::getInstance().getTableColumns(name));
    t->loadFromFile();
    tables[name] = t;
    return t;
}

bool StorageManager::dropTable(const std::string& name) {
    return tables.erase(name) > 0;
}

std::vector<std::string> StorageManager::listTables() const {
    std::vector<std::string> names;
    for (const auto& kv : tables) names.push_back(kv.first);
    return names;
} 