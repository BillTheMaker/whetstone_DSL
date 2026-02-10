cmake_minimum_required(VERSION 3.20)
project(WhetstoneEditor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

if(MSVC)
    add_compile_options(/FS /bigobj)
endif()

# Use vcpkg-installed nlohmann_json instead of FetchContent
find_package(nlohmann_json CONFIG REQUIRED)

# Tree-sitter core (from vcpkg)
find_package(unofficial-tree-sitter CONFIG REQUIRED)

# Tree-sitter grammar libraries (not available in vcpkg, fetched from GitHub)
include(FetchContent)

FetchContent_Declare(
    tree-sitter-python
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-python.git
    GIT_TAG        v0.23.6
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    tree-sitter-cpp
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-cpp.git
    GIT_TAG        v0.23.4
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    tree-sitter-elisp
    GIT_REPOSITORY https://github.com/Wilfred/tree-sitter-elisp.git
    GIT_TAG        1.3.0
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    tree-sitter-javascript
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-javascript.git
    GIT_TAG        v0.23.1
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    tree-sitter-typescript
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-typescript.git
    GIT_TAG        v0.23.2
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    tree-sitter-java
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-java.git
    GIT_TAG        v0.23.5
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    tree-sitter-rust
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-rust.git
    GIT_TAG        v0.23.3
    GIT_SHALLOW    TRUE
    SOURCE_DIR     ${CMAKE_BINARY_DIR}/_deps/tree-sitter-rust-src2
    BINARY_DIR     ${CMAKE_BINARY_DIR}/_deps/tree-sitter-rust-build2
)

FetchContent_Declare(
    tree-sitter-go
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-go.git
    GIT_TAG        v0.23.4
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY https://github.com/native-toolkit/tinyfiledialogs.git
    GIT_TAG        2.9.3
    GIT_SHALLOW    TRUE
)

# Populate without add_subdirectory (grammars have their own CMakeLists with conflicting targets)
FetchContent_GetProperties(tree-sitter-python)
if(NOT tree-sitter-python_POPULATED)
    FetchContent_Populate(tree-sitter-python)
endif()

FetchContent_GetProperties(tree-sitter-cpp)
if(NOT tree-sitter-cpp_POPULATED)
    FetchContent_Populate(tree-sitter-cpp)
endif()

FetchContent_GetProperties(tree-sitter-elisp)
if(NOT tree-sitter-elisp_POPULATED)
    FetchContent_Populate(tree-sitter-elisp)
endif()

FetchContent_GetProperties(tree-sitter-javascript)
if(NOT tree-sitter-javascript_POPULATED)
    FetchContent_Populate(tree-sitter-javascript)
endif()

FetchContent_GetProperties(tree-sitter-typescript)
if(NOT tree-sitter-typescript_POPULATED)
    FetchContent_Populate(tree-sitter-typescript)
endif()

FetchContent_GetProperties(tree-sitter-java)
if(NOT tree-sitter-java_POPULATED)
    FetchContent_Populate(tree-sitter-java)
endif()

FetchContent_GetProperties(tree-sitter-rust)
if(NOT tree-sitter-rust_POPULATED)
    FetchContent_Populate(tree-sitter-rust)
endif()

FetchContent_GetProperties(tree-sitter-go)
if(NOT tree-sitter-go_POPULATED)
    FetchContent_Populate(tree-sitter-go)
endif()

FetchContent_GetProperties(tinyfiledialogs)
if(NOT tinyfiledialogs_POPULATED)
    FetchContent_Populate(tinyfiledialogs)
endif()

# Build tree-sitter-python grammar as a static C library
add_library(tree_sitter_python STATIC
    ${tree-sitter-python_SOURCE_DIR}/src/parser.c
    ${tree-sitter-python_SOURCE_DIR}/src/scanner.c
)
target_include_directories(tree_sitter_python PUBLIC
    ${tree-sitter-python_SOURCE_DIR}/src
)
target_link_libraries(tree_sitter_python PUBLIC unofficial::tree-sitter::tree-sitter)

# Build tree-sitter-cpp grammar as a static C library
# Note: tree-sitter-cpp scanner is C++ (.cc)
add_library(tree_sitter_cpp STATIC
    ${tree-sitter-cpp_SOURCE_DIR}/src/parser.c
    ${tree-sitter-cpp_SOURCE_DIR}/src/scanner.c
)
target_include_directories(tree_sitter_cpp PUBLIC
    ${tree-sitter-cpp_SOURCE_DIR}/src
)
target_link_libraries(tree_sitter_cpp PUBLIC unofficial::tree-sitter::tree-sitter)

# Build tree-sitter-elisp grammar as a static C library
add_library(tree_sitter_elisp STATIC
    ${tree-sitter-elisp_SOURCE_DIR}/src/parser.c
)
target_include_directories(tree_sitter_elisp PUBLIC
    ${tree-sitter-elisp_SOURCE_DIR}/src
)
target_link_libraries(tree_sitter_elisp PUBLIC unofficial::tree-sitter::tree-sitter)

# Build tree-sitter-javascript grammar as a static C library
set(_ts_js_src ${tree-sitter-javascript_SOURCE_DIR}/src/parser.c)
if(EXISTS "${tree-sitter-javascript_SOURCE_DIR}/src/scanner.c")
    list(APPEND _ts_js_src "${tree-sitter-javascript_SOURCE_DIR}/src/scanner.c")
endif()
add_library(tree_sitter_javascript STATIC ${_ts_js_src})
target_include_directories(tree_sitter_javascript PUBLIC ${tree-sitter-javascript_SOURCE_DIR}/src)
target_link_libraries(tree_sitter_javascript PUBLIC unofficial::tree-sitter::tree-sitter)

# Build tree-sitter-typescript grammar as a static C library (typescript only)
set(_ts_ts_src ${tree-sitter-typescript_SOURCE_DIR}/typescript/src/parser.c)
if(EXISTS "${tree-sitter-typescript_SOURCE_DIR}/typescript/src/scanner.c")
    list(APPEND _ts_ts_src "${tree-sitter-typescript_SOURCE_DIR}/typescript/src/scanner.c")
endif()
add_library(tree_sitter_typescript STATIC ${_ts_ts_src})
target_include_directories(tree_sitter_typescript PUBLIC ${tree-sitter-typescript_SOURCE_DIR}/typescript/src)
target_link_libraries(tree_sitter_typescript PUBLIC unofficial::tree-sitter::tree-sitter)

# Build tree-sitter-java grammar as a static C library
set(_ts_java_src ${tree-sitter-java_SOURCE_DIR}/src/parser.c)
if(EXISTS "${tree-sitter-java_SOURCE_DIR}/src/scanner.c")
    list(APPEND _ts_java_src "${tree-sitter-java_SOURCE_DIR}/src/scanner.c")
endif()
add_library(tree_sitter_java STATIC ${_ts_java_src})
target_include_directories(tree_sitter_java PUBLIC ${tree-sitter-java_SOURCE_DIR}/src)
target_link_libraries(tree_sitter_java PUBLIC unofficial::tree-sitter::tree-sitter)

# Build tree-sitter-rust grammar as a static C library
set(_ts_rust_src ${tree-sitter-rust_SOURCE_DIR}/src/parser.c)
if(EXISTS "${tree-sitter-rust_SOURCE_DIR}/src/scanner.c")
    list(APPEND _ts_rust_src "${tree-sitter-rust_SOURCE_DIR}/src/scanner.c")
endif()
add_library(tree_sitter_rust STATIC ${_ts_rust_src})
target_include_directories(tree_sitter_rust PUBLIC ${tree-sitter-rust_SOURCE_DIR}/src)
target_link_libraries(tree_sitter_rust PUBLIC unofficial::tree-sitter::tree-sitter)

# Build tree-sitter-go grammar as a static C library
set(_ts_go_src ${tree-sitter-go_SOURCE_DIR}/src/parser.c)
if(EXISTS "${tree-sitter-go_SOURCE_DIR}/src/scanner.c")
    list(APPEND _ts_go_src "${tree-sitter-go_SOURCE_DIR}/src/scanner.c")
endif()
add_library(tree_sitter_go STATIC ${_ts_go_src})
target_include_directories(tree_sitter_go PUBLIC ${tree-sitter-go_SOURCE_DIR}/src)
target_link_libraries(tree_sitter_go PUBLIC unofficial::tree-sitter::tree-sitter)

# tinyfiledialogs (single C file)
add_library(tinyfiledialogs STATIC ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c)
target_include_directories(tinyfiledialogs PUBLIC ${tinyfiledialogs_SOURCE_DIR})

add_executable(step1_test tests/step1_test.cpp)
target_include_directories(step1_test PRIVATE src)

add_executable(step2_test tests/step2_test.cpp)
target_include_directories(step2_test PRIVATE src)

add_executable(step3_test tests/step3_test.cpp)
target_include_directories(step3_test PRIVATE src)

add_executable(step4_test tests/step4_test.cpp)
target_include_directories(step4_test PRIVATE src)

add_executable(step5_test tests/step5_test.cpp)
target_include_directories(step5_test PRIVATE src)
target_link_libraries(step5_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step6_test tests/step6_test.cpp)
target_include_directories(step6_test PRIVATE src)
target_link_libraries(step6_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step7_test tests/step7_test.cpp)
target_include_directories(step7_test PRIVATE src)

add_executable(step8_test tests/step8_test.cpp)
target_include_directories(step8_test PRIVATE src)

add_executable(step9_test tests/step9_test.cpp)
target_include_directories(step9_test PRIVATE src)

add_executable(step10_test tests/step10_test.cpp)
target_include_directories(step10_test PRIVATE src)

add_executable(step11_test tests/step11_test.cpp)
target_include_directories(step11_test PRIVATE src)

add_executable(step12_test tests/step12_test.cpp)
target_include_directories(step12_test PRIVATE src)

add_executable(step13_test tests/step13_test.cpp)
target_include_directories(step13_test PRIVATE src)

add_executable(step14_test tests/step14_test.cpp)
target_include_directories(step14_test PRIVATE src)

add_executable(step15_test tests/step15_test.cpp)
target_include_directories(step15_test PRIVATE src)

add_executable(step16_test tests/step16_test.cpp)
target_include_directories(step16_test PRIVATE src)

add_executable(step17_test tests/step17_test.cpp)
target_include_directories(step17_test PRIVATE src)

add_executable(step18_test tests/step18_test.cpp)
target_include_directories(step18_test PRIVATE src)

add_executable(step19_test tests/step19_test.cpp)
target_include_directories(step19_test PRIVATE src)

add_executable(step20_test tests/step20_test.cpp)
target_include_directories(step20_test PRIVATE src)

add_executable(step21_test tests/step21_test.cpp)
target_include_directories(step21_test PRIVATE src)

add_executable(step22_test tests/step22_test.cpp)
target_include_directories(step22_test PRIVATE src)

add_executable(step23_test tests/step23_test.cpp)
target_include_directories(step23_test PRIVATE src)

add_executable(step24_test tests/step24_test.cpp)
target_include_directories(step24_test PRIVATE src)

add_executable(step25_test tests/step25_test.cpp)
target_include_directories(step25_test PRIVATE src)

add_executable(step26_test tests/step26_test.cpp)
target_include_directories(step26_test PRIVATE src)

add_executable(step27_test tests/step27_test.cpp)
target_include_directories(step27_test PRIVATE src)

add_executable(step28_test tests/step28_test.cpp)
target_include_directories(step28_test PRIVATE src)

add_executable(step29_test tests/step29_test.cpp)
target_include_directories(step29_test PRIVATE src)

add_executable(step30_test tests/step30_test.cpp)
target_include_directories(step30_test PRIVATE src)

add_executable(step31_test tests/step31_test.cpp)
target_include_directories(step31_test PRIVATE src)

add_executable(step32_test tests/step32_test.cpp)
target_include_directories(step32_test PRIVATE src)

add_executable(step33_test tests/step33_test.cpp)
target_include_directories(step33_test PRIVATE src)

add_executable(step34_test tests/step34_test.cpp)
target_include_directories(step34_test PRIVATE src)

add_executable(step35_test tests/step35_test.cpp)
target_include_directories(step35_test PRIVATE src)

add_executable(step36_test tests/step36_test.cpp)
target_include_directories(step36_test PRIVATE src)

add_executable(step37_test tests/step37_test.cpp)
target_include_directories(step37_test PRIVATE src)

add_executable(step38_test tests/step38_test.cpp)
target_include_directories(step38_test PRIVATE src)

add_executable(step39_test tests/step39_test.cpp)
target_include_directories(step39_test PRIVATE src)

add_executable(step40_test tests/step40_test.cpp)
target_include_directories(step40_test PRIVATE src)

add_executable(step41_test tests/step41_test.cpp)
target_include_directories(step41_test PRIVATE src)

add_executable(step42_test tests/step42_test.cpp)
target_include_directories(step42_test PRIVATE src)

add_executable(step43_test tests/step43_test.cpp)
target_include_directories(step43_test PRIVATE src)

add_executable(step44_test tests/step44_test.cpp)
target_include_directories(step44_test PRIVATE src)

add_executable(step45_test tests/step45_test.cpp)
target_include_directories(step45_test PRIVATE src)
target_link_libraries(step45_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_python)

add_executable(step46_test tests/step46_test.cpp)
target_include_directories(step46_test PRIVATE src)
target_link_libraries(step46_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_cpp)

add_executable(step47_test tests/step47_test.cpp)
target_include_directories(step47_test PRIVATE src)
target_link_libraries(step47_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_elisp)

add_executable(step48_test tests/step48_test.cpp)
target_include_directories(step48_test PRIVATE src)
target_link_libraries(step48_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step49_test tests/step49_test.cpp)
target_include_directories(step49_test PRIVATE src)
target_link_libraries(step49_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step50_test tests/step50_test.cpp)
target_include_directories(step50_test PRIVATE src)
target_link_libraries(step50_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step51_test tests/step51_test.cpp)
target_include_directories(step51_test PRIVATE src)
target_link_libraries(step51_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step52_test tests/step52_test.cpp)
target_include_directories(step52_test PRIVATE src)
target_link_libraries(step52_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)

add_executable(step53_test tests/step53_test.cpp)
target_include_directories(step53_test PRIVATE src)
target_link_libraries(step53_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step54_test tests/step54_test.cpp)
target_include_directories(step54_test PRIVATE src)

add_executable(step55_test tests/step55_test.cpp)
target_include_directories(step55_test PRIVATE src)

add_executable(step56_test tests/step56_test.cpp)
target_include_directories(step56_test PRIVATE src)

add_executable(step57_test tests/step57_test.cpp)
target_include_directories(step57_test PRIVATE src)

add_executable(step58_test tests/step58_test.cpp)
target_include_directories(step58_test PRIVATE src)

add_executable(step59_test tests/step59_test.cpp)
target_include_directories(step59_test PRIVATE src)
target_link_libraries(step59_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step60_test tests/step60_test.cpp)
target_include_directories(step60_test PRIVATE src)
target_link_libraries(step60_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step61_test tests/step61_test.cpp)
target_include_directories(step61_test PRIVATE src)

add_executable(step62_test tests/step62_test.cpp)
target_include_directories(step62_test PRIVATE src)

add_executable(step63_test tests/step63_test.cpp)
target_include_directories(step63_test PRIVATE src)

add_executable(step64_test tests/step64_test.cpp)
target_include_directories(step64_test PRIVATE src)

add_executable(step65_test tests/step65_test.cpp)
target_include_directories(step65_test PRIVATE src)

add_executable(step66_test tests/step66_test.cpp)
target_include_directories(step66_test PRIVATE src)

add_executable(step67_test tests/step67_test.cpp)
target_include_directories(step67_test PRIVATE src)

add_executable(step68_test tests/step68_test.cpp)
target_include_directories(step68_test PRIVATE src)

add_executable(step69_test tests/step69_test.cpp)
target_include_directories(step69_test PRIVATE src)

add_executable(step70_test tests/step70_test.cpp)
target_include_directories(step70_test PRIVATE src)

add_executable(step71_test tests/step71_test.cpp)
target_include_directories(step71_test PRIVATE src)

add_executable(step72_test tests/step72_test.cpp)
target_include_directories(step72_test PRIVATE src)
target_link_libraries(step72_test PRIVATE unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step73_test tests/step73_test.cpp)
target_include_directories(step73_test PRIVATE src)

add_executable(step74_test tests/step74_test.cpp)
target_include_directories(step74_test PRIVATE src)
target_link_libraries(step74_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step75_test tests/step75_test.cpp)
target_include_directories(step75_test PRIVATE src)

add_executable(step76_test tests/step76_test.cpp)
target_include_directories(step76_test PRIVATE src)

add_executable(step77_test tests/step77_test.cpp)
target_include_directories(step77_test PRIVATE src)
target_link_libraries(step77_test PRIVATE
    imgui::imgui
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp)

add_executable(step78_test tests/step78_test.cpp)
target_include_directories(step78_test PRIVATE src)
target_link_libraries(step78_test PRIVATE
    imgui::imgui
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp)

add_executable(step79_test tests/step79_test.cpp)
target_include_directories(step79_test PRIVATE src)
target_link_libraries(step79_test PRIVATE
    imgui::imgui
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp)

add_executable(step80_test tests/step80_test.cpp)
target_include_directories(step80_test PRIVATE src)
target_link_libraries(step80_test PRIVATE
    imgui::imgui
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp)

add_executable(step81_test tests/step81_test.cpp)
target_include_directories(step81_test PRIVATE src)
target_link_libraries(step81_test PRIVATE
    imgui::imgui
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp)

add_executable(step82_test tests/step82_test.cpp)
target_include_directories(step82_test PRIVATE src)
target_link_libraries(step82_test PRIVATE
    imgui::imgui
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp)

add_executable(step83_test tests/step83_test.cpp)
target_include_directories(step83_test PRIVATE src)
target_link_libraries(step83_test PRIVATE unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)

add_executable(step84_test tests/step84_test.cpp src/FileDialog.cpp)
target_include_directories(step84_test PRIVATE src)
target_link_libraries(step84_test PRIVATE tinyfiledialogs)

add_executable(step85_test tests/step85_test.cpp)
target_include_directories(step85_test PRIVATE src)

add_executable(step86_test tests/step86_test.cpp)
target_include_directories(step86_test PRIVATE src)

add_executable(step87_test tests/step87_test.cpp)
target_include_directories(step87_test PRIVATE src)

add_executable(step88_test tests/step88_test.cpp)
target_include_directories(step88_test PRIVATE src)

add_executable(step89_test tests/step89_test.cpp)
target_include_directories(step89_test PRIVATE src)

add_executable(step90_test tests/step90_test.cpp)
target_include_directories(step90_test PRIVATE src)
target_link_libraries(step90_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step91_test tests/step91_test.cpp)
target_include_directories(step91_test PRIVATE src)
target_link_libraries(step91_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step92_test tests/step92_test.cpp)
target_include_directories(step92_test PRIVATE src)
target_link_libraries(step92_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step93_test tests/step93_test.cpp)
target_include_directories(step93_test PRIVATE src)
target_link_libraries(step93_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step94_test tests/step94_test.cpp)
target_include_directories(step94_test PRIVATE src)
target_link_libraries(step94_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step94a_test tests/step94a_test.cpp)
target_include_directories(step94a_test PRIVATE src)
target_link_libraries(step94a_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python)

add_executable(step95_test tests/step95_test.cpp)
target_include_directories(step95_test PRIVATE src)
target_link_libraries(step95_test PRIVATE imgui::imgui unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step96_test tests/step96_test.cpp)
target_include_directories(step96_test PRIVATE src)
target_link_libraries(step96_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step97_test tests/step97_test.cpp)
target_include_directories(step97_test PRIVATE src)
target_link_libraries(step97_test PRIVATE imgui::imgui unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step98_test tests/step98_test.cpp)
target_include_directories(step98_test PRIVATE src)
target_link_libraries(step98_test PRIVATE imgui::imgui unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp)

add_executable(step99_test tests/step99_test.cpp)
target_include_directories(step99_test PRIVATE src)
target_link_libraries(step99_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step100_test tests/step100_test.cpp)
target_include_directories(step100_test PRIVATE src)
target_link_libraries(step100_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step101_test tests/step101_test.cpp)
target_include_directories(step101_test PRIVATE src)
target_link_libraries(step101_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step102_test tests/step102_test.cpp)
target_include_directories(step102_test PRIVATE src)
target_link_libraries(step102_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step103_test tests/step103_test.cpp)
target_include_directories(step103_test PRIVATE src)
target_link_libraries(step103_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step104_test tests/step104_test.cpp)
target_include_directories(step104_test PRIVATE src)
target_link_libraries(step104_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step105_test tests/step105_test.cpp)
target_include_directories(step105_test PRIVATE src)
target_link_libraries(step105_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step106_test tests/step106_test.cpp)
target_include_directories(step106_test PRIVATE src)
target_link_libraries(step106_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step107_test tests/step107_test.cpp)
target_include_directories(step107_test PRIVATE src)
target_link_libraries(step107_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step107a_test tests/step107a_test.cpp)
target_include_directories(step107a_test PRIVATE src)
target_link_libraries(step107a_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step107b_test tests/step107b_test.cpp)
target_include_directories(step107b_test PRIVATE src)
target_link_libraries(step107b_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step107c_test tests/step107c_test.cpp)
target_include_directories(step107c_test PRIVATE src)
target_link_libraries(step107c_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step108_test tests/step108_test.cpp)
target_include_directories(step108_test PRIVATE src)
target_link_libraries(step108_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step109_test tests/step109_test.cpp)
target_include_directories(step109_test PRIVATE src)
target_link_libraries(step109_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step110_test tests/step110_test.cpp)
target_include_directories(step110_test PRIVATE src)
target_link_libraries(step110_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step111_test tests/step111_test.cpp)
target_include_directories(step111_test PRIVATE src)
target_link_libraries(step111_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step112_test tests/step112_test.cpp)
target_include_directories(step112_test PRIVATE src)
target_link_libraries(step112_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step113_test tests/step113_test.cpp)
target_include_directories(step113_test PRIVATE src)
target_link_libraries(step113_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step114_test tests/step114_test.cpp)
target_include_directories(step114_test PRIVATE src)
target_link_libraries(step114_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step115_test tests/step115_test.cpp)
target_include_directories(step115_test PRIVATE src)
target_link_libraries(step115_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step116_test tests/step116_test.cpp)
target_include_directories(step116_test PRIVATE src)
target_link_libraries(step116_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step117_test tests/step117_test.cpp)
target_include_directories(step117_test PRIVATE src)
target_link_libraries(step117_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step118_test tests/step118_test.cpp)
target_include_directories(step118_test PRIVATE src)
target_link_libraries(step118_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step119_test tests/step119_test.cpp)
target_include_directories(step119_test PRIVATE src)
target_link_libraries(step119_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step120_test tests/step120_test.cpp)
target_include_directories(step120_test PRIVATE src)
target_link_libraries(step120_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step121_test tests/step121_test.cpp)
target_include_directories(step121_test PRIVATE src)
target_link_libraries(step121_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step122_test tests/step122_test.cpp)
target_include_directories(step122_test PRIVATE src)
target_link_libraries(step122_test PRIVATE imgui::imgui)

add_executable(step123_test tests/step123_test.cpp)
target_include_directories(step123_test PRIVATE src)
target_link_libraries(step123_test PRIVATE imgui::imgui)

add_executable(step124_test tests/step124_test.cpp)
target_include_directories(step124_test PRIVATE src)
target_link_libraries(step124_test PRIVATE
    nlohmann_json::nlohmann_json)

add_executable(step125_test tests/step125_test.cpp)
target_include_directories(step125_test PRIVATE src)
target_link_libraries(step125_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step125_integration_test tests/step125_integration_test.cpp)
target_include_directories(step125_integration_test PRIVATE src)
target_link_libraries(step125_integration_test PRIVATE
    imgui::imgui
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp)

add_executable(step126_test tests/step126_test.cpp)
target_include_directories(step126_test PRIVATE src)
target_link_libraries(step126_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step127_test tests/step127_test.cpp)
target_include_directories(step127_test PRIVATE src)
target_link_libraries(step127_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step128_test tests/step128_test.cpp)
target_include_directories(step128_test PRIVATE src)
target_link_libraries(step128_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step129_test tests/step129_test.cpp)
target_include_directories(step129_test PRIVATE src)
target_link_libraries(step129_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step130_test tests/step130_test.cpp)
target_include_directories(step130_test PRIVATE src)
target_link_libraries(step130_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step131_test tests/step131_test.cpp)
target_include_directories(step131_test PRIVATE src)
target_link_libraries(step131_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step132_test tests/step132_test.cpp)
target_include_directories(step132_test PRIVATE src)
target_link_libraries(step132_test PRIVATE
    imgui::imgui
    nlohmann_json::nlohmann_json)

add_executable(step133_test tests/step133_test.cpp)
target_include_directories(step133_test PRIVATE src)
target_link_libraries(step133_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step134_test tests/step134_test.cpp)
target_include_directories(step134_test PRIVATE src)
target_link_libraries(step134_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step135_test tests/step135_test.cpp)
target_include_directories(step135_test PRIVATE src)
target_link_libraries(step135_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step136_test tests/step136_test.cpp)
target_include_directories(step136_test PRIVATE src)
target_link_libraries(step136_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step137_test tests/step137_test.cpp)
target_include_directories(step137_test PRIVATE src)
target_link_libraries(step137_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step138_test tests/step138_test.cpp)
target_include_directories(step138_test PRIVATE src)
target_link_libraries(step138_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step139_test tests/step139_test.cpp)
target_include_directories(step139_test PRIVATE src)
target_link_libraries(step139_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step140_test tests/step140_test.cpp)
target_include_directories(step140_test PRIVATE src)
target_link_libraries(step140_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step141_test tests/step141_test.cpp)
target_include_directories(step141_test PRIVATE src)
target_link_libraries(step141_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step142_test tests/step142_test.cpp)
target_include_directories(step142_test PRIVATE src)
target_link_libraries(step142_test PRIVATE nlohmann_json::nlohmann_json imgui::imgui)

find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# imgui SDL2 backend: vcpkg removed the sdl2-binding feature in imgui 1.91+.
# We compile the backend sources directly from imgui's source tree.
set(IMGUI_BACKENDS_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
set(IMGUI_SDL2_BACKEND "")
if(EXISTS "${IMGUI_BACKENDS_DIR}/imgui_impl_sdl2.h")
    # vcpkg installed the SDL2 backend (older vcpkg or feature restored)
    message(STATUS "Using vcpkg-installed imgui SDL2 backend")
else()
    # Compile SDL2 backend from imgui source (shipped in backends/ of the port)
    file(GLOB _imgui_sdl2_src
        "${_VCPKG_INSTALLED_DIR}/../buildtrees/imgui/src/*/backends/imgui_impl_sdl2.cpp"
    )
    if(NOT _imgui_sdl2_src)
        # Fallback: look for the backend in a local copy
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_backends/imgui_impl_sdl2.cpp")
            set(_imgui_sdl2_src "${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_backends/imgui_impl_sdl2.cpp")
        endif()
    endif()
    if(_imgui_sdl2_src)
        list(GET _imgui_sdl2_src 0 _imgui_sdl2_src)
        get_filename_component(_imgui_sdl2_dir "${_imgui_sdl2_src}" DIRECTORY)
        message(STATUS "Compiling imgui SDL2 backend from: ${_imgui_sdl2_dir}")
        set(IMGUI_SDL2_BACKEND "${_imgui_sdl2_src}")
    else()
        message(WARNING "imgui SDL2 backend not found. Editor will not compile. "
            "Copy imgui_impl_sdl2.cpp/.h into editor/src/imgui_backends/")
    endif()
endif()

add_executable(whetstone_editor src/main.cpp src/FileDialog.cpp ${IMGUI_SDL2_BACKEND})
target_include_directories(whetstone_editor PRIVATE
    src
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/imgui
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
)
if(IMGUI_SDL2_BACKEND)
    get_filename_component(_sdl2_backend_dir "${IMGUI_SDL2_BACKEND}" DIRECTORY)
    target_include_directories(whetstone_editor PRIVATE "${_sdl2_backend_dir}")
endif()
target_link_libraries(whetstone_editor PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    OpenGL::GL
    glad::glad
    imgui::imgui
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)
target_link_libraries(whetstone_editor PRIVATE tinyfiledialogs)

add_executable(orchestrator src/orchestrator_main.cpp)
target_include_directories(orchestrator PRIVATE src)
target_link_libraries(orchestrator PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)

# Step 12: Dear ImGui shell scaffolding created (main.cpp exists but not built due to dependencies)
