cmake_minimum_required(VERSION 3.16)
project(Automatic_Differentiation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For IDE jump-to-definition (clangd / C++ extension).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(ad STATIC
  src/expr.cpp
  src/nodes.cpp
  src/derivative.cpp
  src/evaluator.cpp
  src/ops.cpp
)

target_include_directories(ad PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(ad PRIVATE -Wall -Wextra -Wpedantic)

add_executable(ad_example examples/main.cpp)
target_link_libraries(ad_example PRIVATE ad)
target_include_directories(ad_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()
add_executable(ad_test tests/test_ad.cpp)
target_link_libraries(ad_test PRIVATE ad)
target_include_directories(ad_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_test(NAME ad_test COMMAND ad_test)
