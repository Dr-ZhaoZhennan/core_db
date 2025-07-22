#pragma once
#include <memory>
#include "../parser/SQLStatement.h"
#include "Plan.h"

class Optimizer {
public:
    // 输入SQL语句，输出执行计划
    static std::shared_ptr<Plan> optimize(const std::shared_ptr<SQLStatement>& stmt);
}; 