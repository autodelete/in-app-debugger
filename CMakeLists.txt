cmake_minimum_required(VERSION 3.10)
project(in_app_debugger)

set(CMAKE_CXX_STANDARD 14)

add_executable(in_app_debugger main.cpp InAppDebugger.cpp InAppDebugger.h InAppDebuggerCallbacks.h Thread.cpp Thread.h Process.cpp Process.h syscall_abi.h)

target_link_libraries(in_app_debugger pthread)