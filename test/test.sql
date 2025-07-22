-- 创建测试表
CREATE TABLE users (id INT, name TEXT, age INT);
CREATE TABLE products (id INT, name TEXT, price FLOAT);
CREATE TABLE orders (id INT, user_id INT, product_id INT, quantity INT);
CREATE TABLE employees (id INT, name TEXT, department_id INT);
CREATE TABLE departments (id INT, name TEXT);

-- 插入测试数据
INSERT INTO users VALUES (1, 'Alice', 30);
INSERT INTO users VALUES (2, 'Bob', 25);
INSERT INTO products VALUES (1, 'Laptop', 5999.99);
INSERT INTO products VALUES (2, 'Phone', 3999.99);
INSERT INTO orders VALUES (1, 1, 2, 1);
INSERT INTO employees VALUES (1, 'Tom', 1);
INSERT INTO departments VALUES (1, 'HR'); 