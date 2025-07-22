cmake_minimum_required(VERSION 3.10)
project(MyDB)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/optimizer/*.cpp")

add_executable(mydb ${SOURCES}) 