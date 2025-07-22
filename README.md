# MyDB 数据库内核项目（详细说明）

本项目为类主流数据库的简化版关系型数据库内核，参考MySQL/PostgreSQL/SQLite等模块化设计，使用C++开发，支持丰富的SQL语句、索引、事务、并发等功能。下文将详细介绍各文件夹/模块的功能、主要接口、典型用法和测试方法。

---

## 目录结构与模块功能

```
core_db/
  ├── src/                # 源码目录（各模块分文件夹，见下文详细说明）
  │   ├── main.cpp        # 主程序入口，命令行交互
  │   ├── parser/         # SQL解析器模块（词法、语法、语句对象）
  │   ├── executor/       # SQL执行器模块（分发、执行、事务、并发）
  │   ├── storage/        # 存储引擎模块（表、行、索引、持久化）
  │   ├── catalog/        # 元数据管理模块（表结构、字段信息）
  │   ├── test/           # 单元测试与SQL脚本（可选）
  │   └── ...             # 其它辅助模块
  ├── test/               # 测试SQL脚本（批量测试用）
  ├── tables/             # 表数据文件（自动生成，持久化存储）
  ├── build/              # 编译输出目录
  ├── CMakeLists.txt      # CMake构建配置
  └── README.md           # 项目说明（本文件）
```

---

### 1. src/  源码主目录

#### main.cpp
- 程序入口，命令行交互，循环读取SQL语句，调用SQL解析器和执行器。
- 典型用法：
  ```bash
  ./mydb
  # 然后输入SQL语句，回车执行
  ```

#### parser/  SQL解析器模块
- **SQLTokenizer.h/cpp**：SQL词法分析，将SQL字符串分割为Token序列。
- **SQLParser.h/cpp**：SQL语法分析，将Token序列解析为SQL语句对象（AST）。
- **SQLStatement.h** 及其子类：定义SQL语句对象体系（如CreateTableStatement、InsertStatement、SelectStatement等），每种SQL类型一个类，便于扩展。
- 主要功能：
  - 支持CREATE/INSERT/SELECT/UPDATE/DELETE/CREATE INDEX/BEGIN/COMMIT/ROLLBACK等SQL语句的解析。
  - 支持多条件WHERE、ORDER BY、LIMIT等语法。
- 典型用法：
  ```cpp
  auto stmt = SQLParser::parse("SELECT * FROM users WHERE id=1;");
  ```

#### executor/  SQL执行器模块
- **Executor.h/cpp**：SQL执行分发，调用存储引擎、元数据、事务、并发等模块。
- 主要功能：
  - 根据SQLStatement类型分发到不同的执行逻辑。
  - 支持事务（BEGIN/COMMIT/ROLLBACK）、并发（表级锁）、索引加速。
  - 维护事务快照，支持回滚。
- 典型用法：
  ```cpp
  Executor::execute(stmt); // stmt为SQLParser返回的语句对象
  ```

#### storage/  存储引擎模块
- **Table.h/cpp**：表对象，管理表名、字段、行数据、索引、持久化。
  - 主要接口：insertRow、selectAll、saveToFile、loadFromFile、createIndex、getMutableRows等。
- **Row.h/cpp**：行对象，封装一行数据。
- **StorageManager.h/cpp**：表对象的统一管理，支持表的创建、查找、删除。
- 主要功能：
  - 表数据持久化到tables/目录，支持自动加载和保存。
  - 支持单字段哈希索引，自动维护。
- 典型用法：
  ```cpp
  auto table = StorageManager::getInstance().getTable("users");
  table->insertRow(Row({"1", "Alice", "30"}));
  table->saveToFile();
  ```

#### catalog/  元数据管理模块
- **CatalogManager.h/cpp**：管理表结构、字段信息。
- 主要功能：
  - 记录每张表的字段名、类型（简化为字符串）。
  - 提供表结构查询、创建、删除等接口。
- 典型用法：
  ```cpp
  CatalogManager::getInstance().createTable("users", {"id", "name", "age"});
  auto cols = CatalogManager::getInstance().getTableColumns("users");
  ```

#### 其它辅助模块
- 可扩展日志、缓冲、优化器、工具类等。

---

### 2. test/  测试SQL脚本目录
- 存放批量测试用的SQL脚本（如test.sql）。
- 典型用法：
  ```bash
  ./mydb < ../test/test.sql
  ```
- 可自定义脚本内容，批量测试建表、插入、查询、事务、索引等功能。

---

### 3. tables/  表数据文件目录
- 所有表数据持久化存储于此，每张表一个文件（如users.tbl）。
- 文件内容为行式存储，字段用|分隔。
- 典型用法：
  ```bash
  cat tables/users.tbl
  # 输出：1|Alice|30
  ```
- **注意：不要手动编辑表文件，所有操作请通过SQL语句完成。**

---

### 4. build/  编译输出目录
- 存放编译生成的可执行文件（mydb）及中间文件。
- 典型用法：
  ```bash
  cd build
  ./mydb
  ```

---

### 5. CMakeLists.txt  构建配置
- CMake项目配置，自动递归编译src/下所有cpp文件。
- 典型用法：
  ```bash
  mkdir -p build
  cd build
  cmake ..
  make
  ```

---

## 功能说明与使用方法

### 1. SQL支持
- 支持建表、插入、查询、更新、删除、索引、事务、并发等。
- 典型SQL：
  ```sql
  CREATE TABLE users (id, name, age);
  INSERT INTO users VALUES (1, 'Alice', 30);
  SELECT * FROM users WHERE id=1;
  UPDATE users SET age=31 WHERE name='Alice';
  DELETE FROM users WHERE id=1;
  CREATE INDEX idx_user_id ON users (id);
  BEGIN;
  INSERT INTO users VALUES (2, 'Bob', 25);
  ROLLBACK;
  COMMIT;
  SELECT * FROM users WHERE name='Alice' AND age=31 ORDER BY id DESC LIMIT 1;
  ```

### 2. 事务支持
- `BEGIN` 开启事务，自动快照所有表数据。
- `COMMIT` 提交事务，数据永久生效。
- `ROLLBACK` 回滚事务，恢复到BEGIN时状态。
- 典型用法：
  ```sql
  BEGIN;
  INSERT INTO users VALUES (3, 'Tom', 22);
  ROLLBACK;
  SELECT * FROM users;  -- 不会有Tom
  ```

### 3. 索引支持
- 支持单字段哈希索引，自动维护。
- 查询/更新/删除时，WHERE条件命中索引字段自动加速。
- 典型用法：
  ```sql
  CREATE INDEX idx_user_id ON users (id);
  SELECT * FROM users WHERE id=1;
  ```

### 4. 并发支持
- 表级互斥锁，支持多线程并发安全操作。
- 可在主程序中用多线程并发执行SQL。

### 5. 数据持久化
- 所有表数据自动保存在tables/目录下（如users.tbl）。
- 重启程序，数据依然存在。

### 6. 批量测试
- 可将多条SQL写入test/test.sql，批量执行：
  ```bash
  ./mydb < ../test/test.sql
  ```

---

## 常见问题与排查
- **SQL语法错误**：请严格按照README中的SQL格式书写。
- **表不存在/已存在**：请检查表名拼写和建表顺序。
- **表文件未生成**：请确认插入数据后再查表文件，或检查表文件路径。
- **并发冲突**：表操作已加锁，理论不会出现数据竞争。
- **路径问题**：建议始终在build目录下运行./mydb。

---

## 扩展建议
- 支持JOIN、聚合、复合索引、WAL日志、表结构变更等。
- 可根据实际需求继续扩展。

---

如有任何报错或特殊需求，欢迎随时反馈！ 