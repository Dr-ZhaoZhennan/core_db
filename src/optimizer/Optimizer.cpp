#include "Optimizer.h"
#include "Plan.h"

std::shared_ptr<Plan> Optimizer::optimize(const std::shared_ptr<SQLStatement>& stmt) {
    // 简单实现：直接用SQLStatement生成Plan
    return std::make_shared<Plan>(stmt);
} 