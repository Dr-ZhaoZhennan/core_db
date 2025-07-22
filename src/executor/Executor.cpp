#include "Executor.h"
#include "../parser/CreateTableStatement.h"
#include "../parser/InsertStatement.h"
#include "../parser/SelectStatement.h"
#include "../parser/UpdateStatement.h" // Added for UPDATE
#include "../parser/DeleteStatement.h" // Added for DELETE
#include "../storage/StorageManager.h"
#include "../catalog/CatalogManager.h"
#include <iostream>
#include <memory>
#include <mutex> // Added for mutex
#include <algorithm>
#include <cctype>
#include "../utils/StringUtil.h"

bool Executor::inTransaction = false;
std::map<std::string, std::vector<Row>> Executor::tableSnapshots;
std::map<std::string, std::vector<Row>> Executor::tableSnapshotsIndex;

void Executor::beginTransaction() {
    inTransaction = true;
    tableSnapshots.clear();
    // 快照所有表数据
    for (const auto& name : StorageManager::getInstance().listTables()) {
        auto t = StorageManager::getInstance().getTable(name);
        if (t) tableSnapshots[name] = t->selectAll();
    }
    std::cout << "事务开始。\n";
}
void Executor::commitTransaction() {
    inTransaction = false;
    tableSnapshots.clear();
    std::cout << "事务提交。\n";
}
void Executor::rollbackTransaction() {
    // 恢复所有表数据
    for (const auto& kv : tableSnapshots) {
        auto t = StorageManager::getInstance().getTable(kv.first);
        if (t) {
            auto& allRows = t->getMutableRows();
            allRows = kv.second;
            t->saveToFile();
            t->updateIndex();
        }
    }
    inTransaction = false;
    tableSnapshots.clear();
    std::cout << "事务回滚。\n";
}

void Executor::execute(std::shared_ptr<SQLStatement> stmt) {
    if (!stmt) {
        std::cout << "无效的SQL语句。\n";
        return;
    }
    if (stmt->getType() == "BEGIN") {
        beginTransaction();
        return;
    } else if (stmt->getType() == "COMMIT") {
        commitTransaction();
        return;
    } else if (stmt->getType() == "ROLLBACK") {
        rollbackTransaction();
        return;
    }
    if (stmt->getType() == "CREATE_TABLE") {
        auto createStmt = std::dynamic_pointer_cast<CreateTableStatement>(stmt);
        if (!createStmt) return;
        auto& sm = StorageManager::getInstance();
        std::lock_guard<std::mutex> lock(sm.getTable(createStmt->tableName)->mtx);
        if (CatalogManager::getInstance().createTable(createStmt->tableName, createStmt->columns)) {
            sm.createTable(createStmt->tableName);
            std::cout << "表 " << createStmt->tableName << " 创建成功。\n";
        } else {
            std::cout << "表已存在。\n";
        }
    } else if (stmt->getType() == "CREATE_INDEX") {
        struct CreateIndexStatement : public SQLStatement {
            std::string tableName, col;
            std::string getType() const override { return "CREATE_INDEX"; }
        };
        auto idxStmt = std::dynamic_pointer_cast<CreateIndexStatement>(stmt);
        if (!idxStmt) return;
        auto table = StorageManager::getInstance().getTable(idxStmt->tableName);
        if (!table) {
            std::cout << "表不存在。\n";
            return;
        }
        std::lock_guard<std::mutex> lock(table->mtx);
        table->createIndex(idxStmt->col);
        std::cout << "索引创建成功。\n";
        return;
    } else if (stmt->getType() == "INSERT") {
        auto insertStmt = std::dynamic_pointer_cast<InsertStatement>(stmt);
        if (!insertStmt) return;
        auto table = StorageManager::getInstance().getTable(insertStmt->tableName);
        if (!table) {
            std::cout << "表不存在。\n";
            return;
        }
        std::lock_guard<std::mutex> lock(table->mtx);
        Row row(insertStmt->values);
        table->insertRow(row);
        table->saveToFile();
        table->updateIndex();
        std::cout << "插入成功。\n";
    } else if (stmt->getType() == "SELECT") {
        auto selectStmt = std::dynamic_pointer_cast<SelectStatement>(stmt);
        if (!selectStmt) return;
        auto table = StorageManager::getInstance().getTable(selectStmt->tableName);
        if (!table) {
            std::cout << "表不存在。\n";
            return;
        }
        std::lock_guard<std::mutex> lock(table->mtx);
        auto cols = table->getColumns();
        for (const auto& c : cols) std::cout << c << " ";
        std::cout << std::endl;
        auto& allRows = table->getMutableRows();
        // 多条件过滤
        std::vector<Row> filtered;
        for (const auto& row : allRows) {
            const auto& vals = row.getValues();
            bool match = true;
            for (const auto& cond : selectStmt->whereConds) {
                auto it = std::find(cols.begin(), cols.end(), cond.first);
                if (it == cols.end()) { match = false; break; }
                size_t idx = std::distance(cols.begin(), it);
                // 调试输出
                std::cout << "[DEBUG] 比较列: " << cond.first << " 目标值: [" << cond.second << "] 实际值: [" << (idx < vals.size() ? vals[idx] : "越界") << "]" << std::endl;
                if (idx >= vals.size() || StringUtil::trim(vals[idx]) != StringUtil::trim(cond.second)) { match = false; break; }
            }
            if (match) filtered.push_back(row);
        }
        // 排序
        if (!selectStmt->orderByCol.empty()) {
            auto it = std::find(cols.begin(), cols.end(), selectStmt->orderByCol);
            if (it != cols.end()) {
                size_t idx = std::distance(cols.begin(), it);
                std::sort(filtered.begin(), filtered.end(), [&](const Row& a, const Row& b) {
                    const auto& va = a.getValues();
                    const auto& vb = b.getValues();
                    if (idx >= va.size() || idx >= vb.size()) return false;
                    if (selectStmt->orderDesc) return va[idx] > vb[idx];
                    else return va[idx] < vb[idx];
                });
            }
        }
        // LIMIT
        int cnt = 0;
        for (const auto& row : filtered) {
            if (selectStmt->limit != -1 && cnt >= selectStmt->limit) break;
            for (const auto& val : row.getValues()) std::cout << val << " ";
            std::cout << std::endl;
            ++cnt;
        }
        return;
    } else if (stmt->getType() == "UPDATE") {
        auto updateStmt = std::dynamic_pointer_cast<UpdateStatement>(stmt);
        if (!updateStmt) return;
        auto table = StorageManager::getInstance().getTable(updateStmt->tableName);
        if (!table) {
            std::cout << "表不存在。\n";
            return;
        }
        std::lock_guard<std::mutex> lock(table->mtx);
        auto cols = table->getColumns();
        size_t whereIdx = std::string::npos;
        if (!updateStmt->whereCol.empty()) {
            auto it = std::find(cols.begin(), cols.end(), updateStmt->whereCol);
            if (it != cols.end()) whereIdx = std::distance(cols.begin(), it);
        }
        auto& allRows = table->getMutableRows();
        for (auto& row : allRows) {
            auto vals = row.getValues();
            if (whereIdx != std::string::npos) {
                // 调试输出
                std::cout << "[DEBUG] UPDATE 比较列: " << updateStmt->whereCol << " 目标值: [" << updateStmt->whereVal << "] 实际值: [" << (whereIdx < vals.size() ? vals[whereIdx] : "越界") << "]" << std::endl;
                if (whereIdx >= vals.size() || StringUtil::trim(vals[whereIdx]) != StringUtil::trim(updateStmt->whereVal)) continue;
            }
            for (size_t i = 0; i < updateStmt->columns.size(); ++i) {
                auto it = std::find(cols.begin(), cols.end(), updateStmt->columns[i]);
                if (it != cols.end()) {
                    size_t idx = std::distance(cols.begin(), it);
                    if (idx < vals.size()) vals[idx] = updateStmt->values[i];
                }
            }
            row = Row(vals);
        }
        table->saveToFile();
        table->updateIndex();
        std::cout << "更新成功。\n";
    } else if (stmt->getType() == "DELETE") {
        auto delStmt = std::dynamic_pointer_cast<DeleteStatement>(stmt);
        if (!delStmt) return;
        auto table = StorageManager::getInstance().getTable(delStmt->tableName);
        if (!table) {
            std::cout << "表不存在。\n";
            return;
        }
        std::lock_guard<std::mutex> lock(table->mtx);
        auto cols = table->getColumns();
        size_t whereIdx = std::string::npos;
        if (!delStmt->whereCol.empty()) {
            auto it = std::find(cols.begin(), cols.end(), delStmt->whereCol);
            if (it != cols.end()) whereIdx = std::distance(cols.begin(), it);
        }
        auto& allRows = table->getMutableRows();
        if (whereIdx == std::string::npos) {
            allRows.clear();
        } else {
            allRows.erase(std::remove_if(allRows.begin(), allRows.end(), [&](const Row& row) {
                const auto& vals = row.getValues();
                return whereIdx < vals.size() && vals[whereIdx] == delStmt->whereVal;
            }), allRows.end());
        }
        table->saveToFile();
        table->updateIndex();
        std::cout << "删除成功。\n";
    } else {
        std::cout << "暂不支持该SQL类型。\n";
    }
} 