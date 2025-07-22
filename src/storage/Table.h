#pragma once
#include <string>
#include <vector>
#include "Row.h"
#include <unordered_map>
#include <mutex>

class Table {
public:
    Table(const std::string& name);
    bool load();
    bool save();
    bool insertRow(const Row& row);
    std::vector<Row> selectAll() const;
    std::string getName() const;
    bool loadFromFile();
    bool saveToFile();
    void setColumns(const std::vector<std::string>& cols);
    std::vector<std::string> getColumns() const;
    void createIndex(const std::string& col);
    bool hasIndex(const std::string& col) const;
    void updateIndex();
    std::vector<Row> findByIndex(const std::string& col, const std::string& val) const;
    std::vector<Row>& getMutableRows();
    std::mutex mtx;
private:
    std::string name;
    std::vector<Row> rows;
    std::vector<std::string> columns;
    std::unordered_map<std::string, std::unordered_map<std::string, size_t>> indexes; // 字段名->(值->行号)
}; 