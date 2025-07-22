#pragma once
#include <memory>
#include "../parser/SQLStatement.h"
#include <map>
#include <algorithm>
#include <vector>
#include "../storage/Row.h"

class Executor {
private:
    static bool inTransaction;
    static std::map<std::string, std::vector<Row>> tableSnapshots;
    static std::map<std::string, std::vector<Row>> tableSnapshotsIndex;
public:
    static void beginTransaction();
    static void commitTransaction();
    static void rollbackTransaction();
    static void execute(std::shared_ptr<SQLStatement> stmt);
}; 