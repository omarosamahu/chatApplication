cmake_minimum_required (VERSION 3.8)

project ("ChatAPP")

set(CXX_STANDARD_REQUIRED 14)

add_subdirectory(client_unix)
add_subdirectory(unix_server)
