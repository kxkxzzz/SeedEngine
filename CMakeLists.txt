cmake_minimum_required (VERSION 3.27)

set (CMAKE_C_STANDARD 17)
set (CMAKE_CXX_STANDARD 17)

project (SeedEngine)

include_directories("${PROJECT_SOURCE_DIR}/Framework/Common")
include_directories("${PROJECT_SOURCE_DIR}/Framework/Interface")

add_subdirectory(Framework)
add_subdirectory(Empty)
