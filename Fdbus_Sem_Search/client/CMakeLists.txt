cmake_minimum_required(VERSION 3.10)
project(ClientDemo)
set(CXX_STANDARD 11)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ SRC)
include_directories(/usr/local/usr/include) #指定包含之前获得的头文件目录
#指定可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../Demo/)
link_directories(/usr/local/usr/lib) #指定包含之前获得的动态库目录
add_executable(ClientApp ${SRC})
target_link_libraries(ClientApp pthread fdbus fdbus-clib) #链接 fdbus 和 fdbus-clib 两个动态库

