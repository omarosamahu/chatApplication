cmake_minimum_required (VERSION 3.8)

project ("ServerChatAPP")

set(CXX_STANDARD_REQUIRED 14)

add_executable(${PROJECT_NAME} server_multiplixing.cpp)
