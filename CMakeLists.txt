cmake_minimum_required(VERSION 3.0.0)
project(test VERSION 0.1.0)

include_directories(src)
include_directories(/home/huangtianbo/cpp2/cpp/frame/yaml-cpp/include)
add_definitions(-std=c++11)
link_directories(../lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY /home/huangtianbo/cpp2/cpp/learningCmake/lib)
set(LIBRARY_OUTPUT_PATH ../lib)

SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")  

set(LIB_SRC
    ../src/log.cpp
    ../src/Thread.cpp
    ../src/fiberThread.cpp
    ../src/address.cpp
    ../src/epoll.cpp
    ../src/http.cpp
    ../src/blockIOThread.cpp)

set(LIB
    htb
    pthread)
add_library(htb STATIC ${LIB_SRC})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../test)

add_executable(test_log ../test/test_log.cpp)
target_link_libraries(test_log ${LIB})
target_link_libraries(test_log /home/huangtianbo/cpp2/cpp/frame/yaml-cpp/build/libyaml-cpp.a)

add_executable(test_fiber ../test/test_fiber.cpp)
target_link_libraries(test_fiber ${LIB})
target_link_libraries(test_fiber /home/huangtianbo/cpp2/cpp/frame/yaml-cpp/build/libyaml-cpp.a)

add_executable(test_address_cli ../test/test_address_cli.cpp)
target_link_libraries(test_address_cli ${LIB})
target_link_libraries(test_address_cli /home/huangtianbo/cpp2/cpp/frame/yaml-cpp/build/libyaml-cpp.a)

add_executable(test_address_serv ../test/test_address_serv.cpp)
target_link_libraries(test_address_serv ${LIB})
target_link_libraries(test_address_serv /home/huangtianbo/cpp2/cpp/frame/yaml-cpp/build/libyaml-cpp.a)

add_executable(test_threadpool ../test/test_threadpool.cpp)
target_link_libraries(test_threadpool ${LIB})
target_link_libraries(test_threadpool /home/huangtianbo/cpp2/cpp/frame/yaml-cpp/build/libyaml-cpp.a)

add_executable(test_http ../test/test_http.cpp)
target_link_libraries(test_http ${LIB})
target_link_libraries(test_http /home/huangtianbo/cpp2/cpp/frame/yaml-cpp/build/libyaml-cpp.a)

add_executable(test_fiberEpoll ../test/test_fiberEpoll.cpp)
target_link_libraries(test_fiberEpoll ${LIB})
target_link_libraries(test_fiberEpoll /home/huangtianbo/cpp2/cpp/frame/yaml-cpp/build/libyaml-cpp.a)

add_executable(test_blockepoll ../test/test_blockepoll.cpp)
target_link_libraries(test_blockepoll ${LIB})
target_link_libraries(test_blockepoll /home/huangtianbo/cpp2/cpp/frame/yaml-cpp/build/libyaml-cpp.a)

