#include "Table.h"
#include <fstream>
#include <sstream>
#include <algorithm>

Table::Table(const std::string& name) : name(name) {}

bool Table::load() {
    // TODO: 加载表数据
    return true;
}

bool Table::save() {
    // TODO: 保存表数据
    return true;
}

bool Table::insertRow(const Row& row) {
    rows.push_back(row);
    return true;
}

std::vector<Row> Table::selectAll() const {
    return rows;
}

std::vector<Row>& Table::getMutableRows() { return rows; }

std::string Table::getName() const {
    return name;
}

void Table::setColumns(const std::vector<std::string>& cols) {
    columns = cols;
}
std::vector<std::string> Table::getColumns() const {
    return columns;
}

bool Table::loadFromFile() {
    std::ifstream fin("/home/zzn16/core_db/tables/" + name + ".tbl");
    if (!fin.is_open()) return false;
    rows.clear();
    std::string line;
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::vector<std::string> vals;
        std::string val;
        while (std::getline(ss, val, '|')) {
            vals.push_back(val);
        }
        if (!vals.empty()) rows.emplace_back(vals);
    }
    fin.close();
    return true;
}

bool Table::saveToFile() {
    std::ofstream fout("/home/zzn16/core_db/tables/" + name + ".tbl");
    if (!fout.is_open()) return false;
    for (const auto& row : rows) {
        const auto& vals = row.getValues();
        for (size_t i = 0; i < vals.size(); ++i) {
            fout << vals[i];
            if (i + 1 < vals.size()) fout << "|";
        }
        fout << "\n";
    }
    fout.close();
    return true;
}

void Table::createIndex(const std::string& col) {
    indexes[col].clear();
    for (size_t i = 0; i < rows.size(); ++i) {
        auto vals = rows[i].getValues();
        auto it = std::find(columns.begin(), columns.end(), col);
        if (it != columns.end()) {
            size_t idx = std::distance(columns.begin(), it);
            if (idx < vals.size()) indexes[col][vals[idx]] = i;
        }
    }
}
bool Table::hasIndex(const std::string& col) const {
    return indexes.count(col) > 0;
}
void Table::updateIndex() {
    for (auto& kv : indexes) {
        createIndex(kv.first);
    }
}
std::vector<Row> Table::findByIndex(const std::string& col, const std::string& val) const {
    std::vector<Row> result;
    auto it = indexes.find(col);
    if (it != indexes.end()) {
        auto vit = it->second.find(val);
        if (vit != it->second.end()) {
            result.push_back(rows[vit->second]);
        }
    }
    return result;
} 