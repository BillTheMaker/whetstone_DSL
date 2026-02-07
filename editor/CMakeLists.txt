cmake_minimum_required(VERSION 3.20)
project(WhetstoneEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(step1_test tests/step1_test.cpp)
target_include_directories(step1_test PRIVATE src)

add_executable(step2_test tests/step2_test.cpp)
target_include_directories(step2_test PRIVATE src)

add_executable(step3_test tests/step3_test.cpp)
target_include_directories(step3_test PRIVATE src)

add_executable(step4_test tests/step4_test.cpp)
target_include_directories(step4_test PRIVATE src)
