cmake_minimum_required(VERSION 3.10)

project(spdmq)

option(BUILD_SHARED_LIBS "Build with shared so?" OFF)

# 加速二次编译
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message(STATUS "use ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif ()

# 默认为 debug 模式编译（CMAKE_BUILD_TYPE 设置后 CMAKE_LIBRARY_OUTPUT_DIRECTORY 和 CMAKE_RUNTIME_OUTPUT_DIRECTORY 指定不生效）
if (NOT BUILD_TYPE)
    set(BUILD_TYPE Debug)
endif ()

# 编译 release 或者 debug
if (${BUILD_TYPE} STREQUAL "Release")
    # 使用 C++17 标准, 编译优化为 O2 级别, 删除符号信息, 生成与位置无关的代码, 对所有警告零容忍
    set(CMAKE_CXX_FLAGS "-std=c++17 -O2 -s -Wall -fPIC")
else (${BUILD_TYPE} STREQUAL "Debug")
    # 使用 C++17 标准, 编译优化为 O2 级别, 同时给代码带上symbol, 生成与位置无关的代码, 对所有警告零容忍
    set(CMAKE_CXX_FLAGS "-std=c++17 -O2 -g -Wall -fPIC")
endif ()

# 打印 CMAKE_CXX_FLAGS
message(STATUS ${CMAKE_CXX_FLAGS})

# 包含头文件
include_directories(./)
include_directories(./spdmq)
include_directories(./src)
include_directories(./src/mode)
include_directories(./src/common)
include_directories(./src/components/socket)
include_directories(./src/components/event)
include_directories(./src/components/factory)
include_directories(./src/components/company)

# 编译库文件
file(GLOB_RECURSE spdmq_SRC 
./src/spdmq.cpp 
./src/spdmq_impl.cpp 
./src/spdmq_def.cpp 
./src/components/socket/spdmq_socket.cpp
./src/components/socket/socket_server.cpp
./src/components/socket/socket_client.cpp
./src/components/socket/tcp_client.cpp
./src/components/socket/tcp_server.cpp
./src/components/socket/uds_client.cpp
./src/components/socket/uds_server.cpp
./src/components/factory/socket_factory.cpp
./src/components/factory/event_factory.cpp
./src/components/factory/mode_factory.cpp
./src/components/event/spdmq_event.cpp
./src/components/event/event_poll.cpp
./src/components/event/spdmq_session.cpp
./src/components/company/dispatcher.cpp
./src/components/company/porter.cpp
./src/components/company/storeroom.cpp
./src/mode/spdmq_mode.cpp
./src/mode/mode_publish.cpp
./src/mode/mode_subscribe.cpp
)
if (BUILD_SHARED_LIBS)
    add_library(spdmq SHARED ${spdmq_SRC})
endif ()
add_library(spdmq ${spdmq_SRC})
target_link_libraries(spdmq pthread)

add_executable(pub example/pub.cpp)
target_link_libraries(pub spdmq)
add_executable(sub example/sub.cpp)
target_link_libraries(sub spdmq)