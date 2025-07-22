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
#include "../utils/StringUtil.h"
// 辅助函数：去除首尾引号
std::string strip_quotes(const std::string& s) {
    if (s.size() >= 2 && ((s.front() == '\'' && s.back() == '\'') || (s.front() == '"' && s.back() == '"')))
        return s.substr(1, s.size() - 2);
    return s;
}

std::shared_ptr<SQLStatement> SQLParser::parse(const std::string& sql) {
    // 预处理：去除前后空格和末尾分号
    std::string clean_sql = StringUtil::trim(sql);
    if (!clean_sql.empty() && clean_sql.back() == ';') clean_sql.pop_back();
    // 关键字统一大写
    std::string upper_sql = clean_sql;
    std::transform(upper_sql.begin(), upper_sql.end(), upper_sql.begin(), [](unsigned char c){ return std::toupper(c); });
    auto tokens = SQLTokenizer::tokenize(clean_sql);
    if (tokens.empty()) return nullptr;
    std::string type = tokens[0];
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);
    if (type == "CREATE" && tokens.size() > 2 && tokens[1] == "TABLE") {
        std::string tableName = tokens[2];
        std::vector<std::string> columns;
        size_t l = clean_sql.find('('), r = clean_sql.find(')');
        if (l != std::string::npos && r != std::string::npos && r > l) {
            std::string cols = clean_sql.substr(l+1, r-l-1);
            size_t pos = 0;
            while ((pos = cols.find(',')) != std::string::npos) {
                std::string col = cols.substr(0, pos);
                col.erase(std::remove_if(col.begin(), col.end(), ::isspace), col.end());
                columns.push_back(col);
                cols.erase(0, pos + 1);
            }
            if (!cols.empty()) {
                cols.erase(std::remove_if(cols.begin(), cols.end(), ::isspace), cols.end());
                columns.push_back(cols);
            }
        }
        return std::make_shared<CreateTableStatement>(tableName, columns);
    } else if (type == "INSERT" && tokens.size() > 3 && tokens[1] == "INTO") {
        std::string tableName = tokens[2];
        size_t l = upper_sql.find("VALUES");
        if (l != std::string::npos) {
            size_t lpar = clean_sql.find('(', l), rpar = clean_sql.find(')', l);
            if (lpar != std::string::npos && rpar != std::string::npos && rpar > lpar) {
                std::string vals = clean_sql.substr(lpar+1, rpar-lpar-1);
                std::vector<std::string> values;
                size_t pos = 0;
                while ((pos = vals.find(',')) != std::string::npos) {
                    std::string v = vals.substr(0, pos);
                    v = StringUtil::trim(v);
                    v = strip_quotes(v);
                    values.push_back(v);
                    vals.erase(0, pos + 1);
                }
                if (!vals.empty()) {
                    vals = StringUtil::trim(vals);
                    vals = strip_quotes(vals);
                    values.push_back(vals);
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
            std::string c = colstr.substr(0, pos);
            c = StringUtil::trim(c);
            columns.push_back(c);
            colstr.erase(0, pos + 1);
        }
        if (!colstr.empty()) {
            colstr = StringUtil::trim(colstr);
            columns.push_back(colstr);
        }
        std::string tableName = tokens[3];
        auto stmt = std::make_shared<SelectStatement>(tableName, columns);
        // 解析WHERE多条件
        auto wherePos = upper_sql.find("WHERE");
        if (wherePos != std::string::npos) {
            std::string wherePart = clean_sql.substr(wherePos + 5);
            size_t orderByPos = upper_sql.find("ORDER BY");
            size_t limitPos = upper_sql.find("LIMIT");
            if (orderByPos != std::string::npos) wherePart = wherePart.substr(0, orderByPos - wherePos - 5);
            if (limitPos != std::string::npos) wherePart = wherePart.substr(0, limitPos - wherePos - 5);
            // 支持AND连接的等值条件
            size_t start = 0;
            while (start < wherePart.size()) {
                size_t andPos = wherePart.find("AND", start);
                std::string cond = (andPos == std::string::npos) ? wherePart.substr(start) : wherePart.substr(start, andPos - start);
                size_t eq = cond.find('=');
                if (eq != std::string::npos) {
                    std::string col = cond.substr(0, eq);
                    std::string val = cond.substr(eq+1);
                    col = StringUtil::trim(col);
                    val = StringUtil::trim(val);
                    val = strip_quotes(val);
                    stmt->whereConds.push_back({col, val});
                }
                if (andPos == std::string::npos) break;
                start = andPos + 3;
            }
        }
        // 解析ORDER BY
        auto orderByPos = upper_sql.find("ORDER BY");
        if (orderByPos != std::string::npos) {
            std::string orderPart = clean_sql.substr(orderByPos + 8);
            size_t limitPos = upper_sql.find("LIMIT");
            if (limitPos != std::string::npos) orderPart = orderPart.substr(0, limitPos - orderByPos - 8);
            size_t space = orderPart.find_first_of(" ");
            stmt->orderByCol = StringUtil::trim(orderPart.substr(0, space));
            if (orderPart.find("DESC") != std::string::npos) stmt->orderDesc = true;
        }
        // 解析LIMIT
        auto limitPos = upper_sql.find("LIMIT");
        if (limitPos != std::string::npos) {
            std::string lim = clean_sql.substr(limitPos + 5);
            stmt->limit = std::stoi(StringUtil::trim(lim));
        }
        return stmt;
    } else if (type == "UPDATE" && tokens.size() > 3) {
        std::string tableName = tokens[1];
        size_t setPos = upper_sql.find("SET");
        if (setPos != std::string::npos) {
            std::string setPart = clean_sql.substr(setPos + 3);
            size_t wherePos = upper_sql.find("WHERE", setPos);
            std::string whereCol, whereVal;
            if (wherePos != std::string::npos) {
                std::string wherePart = clean_sql.substr(wherePos + 5);
                setPart = setPart.substr(0, wherePos - setPos - 3);
                size_t eq = wherePart.find('=');
                if (eq != std::string::npos) {
                    whereCol = StringUtil::trim(wherePart.substr(0, eq));
                    whereVal = StringUtil::trim(wherePart.substr(eq+1));
                    whereVal = strip_quotes(whereVal);
                }
            }
            std::vector<std::string> columns, values;
            size_t pos = 0;
            while ((pos = setPart.find(',')) != std::string::npos) {
                std::string pair = setPart.substr(0, pos);
                size_t eq = pair.find('=');
                if (eq != std::string::npos) {
                    std::string col = StringUtil::trim(pair.substr(0, eq));
                    std::string val = StringUtil::trim(pair.substr(eq+1));
                    val = strip_quotes(val);
                    columns.push_back(col);
                    values.push_back(val);
                }
                setPart.erase(0, pos + 1);
            }
            if (!setPart.empty()) {
                size_t eq = setPart.find('=');
                if (eq != std::string::npos) {
                    std::string col = StringUtil::trim(setPart.substr(0, eq));
                    std::string val = StringUtil::trim(setPart.substr(eq+1));
                    val = strip_quotes(val);
                    columns.push_back(col);
                    values.push_back(val);
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
        auto wherePos = upper_sql.find("WHERE");
        if (wherePos != std::string::npos) {
            std::string wherePart = clean_sql.substr(wherePos + 5);
            size_t eq = wherePart.find('=');
            if (eq != std::string::npos) {
                whereCol = StringUtil::trim(wherePart.substr(0, eq));
                whereVal = StringUtil::trim(wherePart.substr(eq+1));
                whereVal = strip_quotes(whereVal);
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
        size_t l = clean_sql.find('('), r = clean_sql.find(')');
        std::string col;
        if (l != std::string::npos && r != std::string::npos && r > l) {
            col = clean_sql.substr(l+1, r-l-1);
            col.erase(std::remove_if(col.begin(), col.end(), ::isspace), col.end());
        }
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