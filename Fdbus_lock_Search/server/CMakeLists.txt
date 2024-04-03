cmake_minimum_required(VERSION 3.10)
project(ServerDemo)
set(CXX_STANDARD 11)

# 设置可执行文件输出路径为../Demo/
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../Demo/)

# 指定包含Src目录作为头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src)

# 搜集Src目录下的所有源文件
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c")

# 将搜集到的源文件编译成一个名为ServerApp的可执行文件
add_executable(ServerApp ${SRC_FILES})

# 指定链接的动态库目录
link_directories(/usr/local/usr/lib)

# 链接 fdbus 和 fdbus-clib 两个动态库
target_link_libraries(ServerApp pthread fdbus fdbus-clib)