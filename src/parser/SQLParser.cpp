#include "SQLParser.h"
#include "SQLTokenizer.h"
#include "CreateTableStatement.h"
#include "InsertStatement.h"
#include "SelectStatement.h"
#include "UpdateStatement.h"
#include "DeleteStatement.h"
#include <algorithm>
#include <cctype>
#include "SQLStatement.h"

std::shared_ptr<SQLStatement> SQLParser::parse(const std::string& sql) {
    auto tokens = SQLTokenizer::tokenize(sql);
    if (tokens.empty()) return nullptr;
    std::string type = tokens[0];
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);
    if (type == "CREATE" && tokens.size() > 2 && tokens[1] == "TABLE") {
        std::string tableName = tokens[2];
        std::vector<std::string> columns;
        size_t l = sql.find('('), r = sql.find(')');
        if (l != std::string::npos && r != std::string::npos && r > l) {
            std::string cols = sql.substr(l+1, r-l-1);
            size_t pos = 0;
            while ((pos = cols.find(',')) != std::string::npos) {
                columns.push_back(cols.substr(0, pos));
                cols.erase(0, pos + 1);
            }
            if (!cols.empty()) columns.push_back(cols);
            for (auto& c : columns) {
                c.erase(std::remove_if(c.begin(), c.end(), ::isspace), c.end());
            }
        }
        return std::make_shared<CreateTableStatement>(tableName, columns);
    } else if (type == "INSERT" && tokens.size() > 3 && tokens[1] == "INTO") {
        std::string tableName = tokens[2];
        size_t l = sql.find("VALUES");
        if (l != std::string::npos) {
            size_t lpar = sql.find('(', l), rpar = sql.find(')', l);
            if (lpar != std::string::npos && rpar != std::string::npos && rpar > lpar) {
                std::string vals = sql.substr(lpar+1, rpar-lpar-1);
                std::vector<std::string> values;
                size_t pos = 0;
                while ((pos = vals.find(',')) != std::string::npos) {
                    values.push_back(vals.substr(0, pos));
                    vals.erase(0, pos + 1);
                }
                if (!vals.empty()) values.push_back(vals);
                for (auto& v : values) {
                    v.erase(std::remove_if(v.begin(), v.end(), ::isspace), v.end());
                }
                return std::make_shared<InsertStatement>(tableName, values);
            }
        }
    } else if (type == "SELECT" && tokens.size() > 3 && tokens[2] == "FROM") {
        std::vector<std::string> columns;
        size_t fromPos = 2;
        std::string colstr = tokens[1];
        size_t pos = 0;
        while ((pos = colstr.find(',')) != std::string::npos) {
            columns.push_back(colstr.substr(0, pos));
            colstr.erase(0, pos + 1);
        }
        if (!colstr.empty()) columns.push_back(colstr);
        std::string tableName = tokens[3];
        auto stmt = std::make_shared<SelectStatement>(tableName, columns);
        // 解析WHERE多条件
        auto wherePos = sql.find("WHERE");
        if (wherePos != std::string::npos) {
            std::string wherePart = sql.substr(wherePos + 5);
            size_t orderByPos = wherePart.find("ORDER BY");
            size_t limitPos = wherePart.find("LIMIT");
            if (orderByPos != std::string::npos) wherePart = wherePart.substr(0, orderByPos);
            if (limitPos != std::string::npos) wherePart = wherePart.substr(0, limitPos);
            // 支持AND连接的等值条件
            size_t start = 0;
            while (start < wherePart.size()) {
                size_t andPos = wherePart.find("AND", start);
                std::string cond = (andPos == std::string::npos) ? wherePart.substr(start) : wherePart.substr(start, andPos - start);
                size_t eq = cond.find('=');
                if (eq != std::string::npos) {
                    std::string col = cond.substr(0, eq);
                    std::string val = cond.substr(eq+1);
                    col.erase(std::remove_if(col.begin(), col.end(), ::isspace), col.end());
                    val.erase(std::remove_if(val.begin(), val.end(), ::isspace), val.end());
                    stmt->whereConds.push_back({col, val});
                }
                if (andPos == std::string::npos) break;
                start = andPos + 3;
            }
        }
        // 解析ORDER BY
        auto orderByPos = sql.find("ORDER BY");
        if (orderByPos != std::string::npos) {
            std::string orderPart = sql.substr(orderByPos + 8);
            size_t limitPos = orderPart.find("LIMIT");
            if (limitPos != std::string::npos) orderPart = orderPart.substr(0, limitPos);
            size_t space = orderPart.find_first_of(" ");
            stmt->orderByCol = orderPart.substr(0, space);
            if (orderPart.find("DESC") != std::string::npos) stmt->orderDesc = true;
        }
        // 解析LIMIT
        auto limitPos = sql.find("LIMIT");
        if (limitPos != std::string::npos) {
            std::string lim = sql.substr(limitPos + 5);
            stmt->limit = std::stoi(lim);
        }
        return stmt;
    } else if (type == "UPDATE" && tokens.size() > 3) {
        std::string tableName = tokens[1];
        size_t setPos = sql.find("SET");
        if (setPos != std::string::npos) {
            std::string setPart = sql.substr(setPos + 3);
            size_t wherePos = setPart.find("WHERE");
            std::string whereCol, whereVal;
            if (wherePos != std::string::npos) {
                std::string wherePart = setPart.substr(wherePos + 5);
                setPart = setPart.substr(0, wherePos);
                size_t eq = wherePart.find('=');
                if (eq != std::string::npos) {
                    whereCol = wherePart.substr(0, eq);
                    whereVal = wherePart.substr(eq+1);
                    whereCol.erase(std::remove_if(whereCol.begin(), whereCol.end(), ::isspace), whereCol.end());
                    whereVal.erase(std::remove_if(whereVal.begin(), whereVal.end(), ::isspace), whereVal.end());
                }
            }
            std::vector<std::string> columns, values;
            size_t pos = 0;
            while ((pos = setPart.find(',')) != std::string::npos) {
                std::string pair = setPart.substr(0, pos);
                size_t eq = pair.find('=');
                if (eq != std::string::npos) {
                    columns.push_back(pair.substr(0, eq));
                    values.push_back(pair.substr(eq+1));
                }
                setPart.erase(0, pos + 1);
            }
            if (!setPart.empty()) {
                size_t eq = setPart.find('=');
                if (eq != std::string::npos) {
                    columns.push_back(setPart.substr(0, eq));
                    values.push_back(setPart.substr(eq+1));
                }
            }
            auto stmt = std::make_shared<UpdateStatement>(tableName, columns, values);
            stmt->whereCol = whereCol;
            stmt->whereVal = whereVal;
            return stmt;
        }
    } else if (type == "DELETE" && tokens.size() > 2 && tokens[1] == "FROM") {
        std::string tableName = tokens[2];
        std::string whereCol, whereVal;
        auto wherePos = sql.find("WHERE");
        if (wherePos != std::string::npos) {
            std::string wherePart = sql.substr(wherePos + 5);
            size_t eq = wherePart.find('=');
            if (eq != std::string::npos) {
                whereCol = wherePart.substr(0, eq);
                whereVal = wherePart.substr(eq+1);
                whereCol.erase(std::remove_if(whereCol.begin(), whereCol.end(), ::isspace), whereCol.end());
                whereVal.erase(std::remove_if(whereVal.begin(), whereVal.end(), ::isspace), whereVal.end());
            }
        }
        auto stmt = std::make_shared<DeleteStatement>(tableName);
        stmt->whereCol = whereCol;
        stmt->whereVal = whereVal;
        return stmt;
    } else if (type == "CREATE" && tokens.size() > 2 && tokens[1] == "INDEX") {
        // CREATE INDEX idxname ON tablename (col)
        std::string idxName = tokens[2];
        std::string tableName = tokens[4];
        size_t l = sql.find('('), r = sql.find(')');
        std::string col;
        if (l != std::string::npos && r != std::string::npos && r > l) {
            col = sql.substr(l+1, r-l-1);
            col.erase(std::remove_if(col.begin(), col.end(), ::isspace), col.end());
        }
        // 用SQLStatement基类返回，类型为CREATE_INDEX，参数用tableName和col
        struct CreateIndexStatement : public SQLStatement {
            std::string tableName, col;
            std::string getType() const override { return "CREATE_INDEX"; }
        };
        auto stmt = std::make_shared<CreateIndexStatement>();
        stmt->tableName = tableName;
        stmt->col = col;
        return stmt;
    } else if (type == "BEGIN") {
        struct BeginStatement : public SQLStatement {
            std::string getType() const override { return "BEGIN"; }
        };
        return std::make_shared<BeginStatement>();
    } else if (type == "COMMIT") {
        struct CommitStatement : public SQLStatement {
            std::string getType() const override { return "COMMIT"; }
        };
        return std::make_shared<CommitStatement>();
    } else if (type == "ROLLBACK") {
        struct RollbackStatement : public SQLStatement {
            std::string getType() const override { return "ROLLBACK"; }
        };
        return std::make_shared<RollbackStatement>();
    }
    return nullptr;
} 