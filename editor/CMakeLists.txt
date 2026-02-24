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
    tree-sitter-org
    GIT_REPOSITORY https://github.com/milisims/tree-sitter-org.git
    GIT_TAG        v1.3.1
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

FetchContent_GetProperties(tree-sitter-org)
if(NOT tree-sitter-org_POPULATED)
    FetchContent_Populate(tree-sitter-org)
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

# Build tree-sitter-org grammar as a static C library
set(_ts_org_src ${tree-sitter-org_SOURCE_DIR}/src/parser.c)
if(EXISTS "${tree-sitter-org_SOURCE_DIR}/src/scanner.c")
    list(APPEND _ts_org_src "${tree-sitter-org_SOURCE_DIR}/src/scanner.c")
elseif(EXISTS "${tree-sitter-org_SOURCE_DIR}/src/scanner.cc")
    list(APPEND _ts_org_src "${tree-sitter-org_SOURCE_DIR}/src/scanner.cc")
endif()
add_library(tree_sitter_org STATIC ${_ts_org_src})
target_include_directories(tree_sitter_org PUBLIC ${tree-sitter-org_SOURCE_DIR}/src)
target_link_libraries(tree_sitter_org PUBLIC unofficial::tree-sitter::tree-sitter)

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
target_link_libraries(step50_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)

add_executable(step51_test tests/step51_test.cpp)
target_include_directories(step51_test PRIVATE src)
target_link_libraries(step51_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)

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
target_link_libraries(step72_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)

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
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_rust
    tree_sitter_go
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_org)

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
target_link_libraries(step129_test PRIVATE nlohmann_json::nlohmann_json imgui::imgui)

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

add_executable(step143_test tests/step143_test.cpp)
target_include_directories(step143_test PRIVATE src)
target_link_libraries(step143_test PRIVATE nlohmann_json::nlohmann_json imgui::imgui)

add_executable(step144_test tests/step144_test.cpp)
target_include_directories(step144_test PRIVATE src)
target_link_libraries(step144_test PRIVATE nlohmann_json::nlohmann_json imgui::imgui)

add_executable(step145_test tests/step145_test.cpp)
target_include_directories(step145_test PRIVATE src)
target_link_libraries(step145_test PRIVATE nlohmann_json::nlohmann_json imgui::imgui tree_sitter_org)

add_executable(step146_test tests/step146_test.cpp)
target_include_directories(step146_test PRIVATE src)
target_link_libraries(step146_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step147_test tests/step147_test.cpp)
target_include_directories(step147_test PRIVATE src)
target_link_libraries(step147_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_javascript
    tree_sitter_typescript)

add_executable(step148_test tests/step148_test.cpp)
target_include_directories(step148_test PRIVATE src)
target_link_libraries(step148_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step149_test tests/step149_test.cpp)
target_include_directories(step149_test PRIVATE src)
target_link_libraries(step149_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_java)

add_executable(step150_test tests/step150_test.cpp)
target_include_directories(step150_test PRIVATE src)
target_link_libraries(step150_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_rust)

add_executable(step151_test tests/step151_test.cpp)
target_include_directories(step151_test PRIVATE src)
target_link_libraries(step151_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_go)

add_executable(step152_test tests/step152_test.cpp)
target_include_directories(step152_test PRIVATE src)

add_executable(step153_test tests/step153_test.cpp)
target_include_directories(step153_test PRIVATE src)
target_link_libraries(step153_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)

add_executable(step154_test tests/step154_test.cpp)
target_include_directories(step154_test PRIVATE src)
target_link_libraries(step154_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step155_test tests/step155_test.cpp)
target_include_directories(step155_test PRIVATE src)
target_link_libraries(step155_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step156_test tests/step156_test.cpp)
target_include_directories(step156_test PRIVATE src)

add_executable(step157_test tests/step157_test.cpp)
target_include_directories(step157_test PRIVATE src)

add_executable(step158_test tests/step158_test.cpp)
target_include_directories(step158_test PRIVATE src)
target_link_libraries(step158_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step159_test tests/step159_test.cpp)
target_include_directories(step159_test PRIVATE src)
target_link_libraries(step159_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step160_test tests/step160_test.cpp)
target_include_directories(step160_test PRIVATE src)
target_link_libraries(step160_test PRIVATE nlohmann_json::nlohmann_json imgui::imgui)

add_executable(step161_test tests/step161_test.cpp)
target_include_directories(step161_test PRIVATE src)

add_executable(step162_test tests/step162_test.cpp)
target_include_directories(step162_test PRIVATE src)
target_link_libraries(step162_test PRIVATE imgui::imgui)

add_executable(step163_test tests/step163_test.cpp)
target_include_directories(step163_test PRIVATE src)
target_link_libraries(step163_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step164_test tests/step164_test.cpp)
target_include_directories(step164_test PRIVATE src)
target_link_libraries(step164_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step165_test tests/step165_test.cpp)
target_include_directories(step165_test PRIVATE src)
target_link_libraries(step165_test PRIVATE
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

add_executable(file_limits_test tests/file_limits_test.cpp)
target_include_directories(file_limits_test PRIVATE src)

add_executable(step166_test tests/step166_test.cpp)
target_include_directories(step166_test PRIVATE src)
target_link_libraries(step166_test PRIVATE
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

add_executable(step167_test tests/step167_test.cpp)
target_include_directories(step167_test PRIVATE src)
target_link_libraries(step167_test PRIVATE
    nlohmann_json::nlohmann_json)

add_executable(step168_test tests/step168_test.cpp)
target_include_directories(step168_test PRIVATE src)

add_executable(step168_integration_test tests/step168_integration_test.cpp)
target_include_directories(step168_integration_test PRIVATE src)
target_link_libraries(step168_integration_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go)

add_executable(step169_test tests/step169_test.cpp)
target_include_directories(step169_test PRIVATE src)
target_link_libraries(step169_test PRIVATE imgui::imgui)

add_executable(step169_integration_test tests/step169_integration_test.cpp)
target_include_directories(step169_integration_test PRIVATE src)
target_link_libraries(step169_integration_test PRIVATE imgui::imgui)

add_executable(step170_test tests/step170_test.cpp)
target_include_directories(step170_test PRIVATE src)

add_executable(step170_integration_test tests/step170_integration_test.cpp)
target_include_directories(step170_integration_test PRIVATE src)

add_executable(step171_test tests/step171_test.cpp)
target_include_directories(step171_test PRIVATE src)
target_link_libraries(step171_test PRIVATE imgui::imgui)

add_executable(step171_integration_test tests/step171_integration_test.cpp)
target_include_directories(step171_integration_test PRIVATE src)

add_executable(step172_test tests/step172_test.cpp)
target_include_directories(step172_test PRIVATE src)

add_executable(step172_integration_test tests/step172_integration_test.cpp)
target_include_directories(step172_integration_test PRIVATE src)
target_link_libraries(step172_integration_test PRIVATE imgui::imgui)

add_executable(step173_test tests/step173_test.cpp)
target_include_directories(step173_test PRIVATE src)

add_executable(step173_integration_test tests/step173_integration_test.cpp)
target_include_directories(step173_integration_test PRIVATE src)
target_link_libraries(step173_integration_test PRIVATE imgui::imgui)

add_executable(step174_test tests/step174_test.cpp)
target_include_directories(step174_test PRIVATE src)

add_executable(step174_integration_test tests/step174_integration_test.cpp)
target_include_directories(step174_integration_test PRIVATE src)
target_link_libraries(step174_integration_test PRIVATE imgui::imgui)

add_executable(step175_test tests/step175_test.cpp)
target_include_directories(step175_test PRIVATE src)

add_executable(step175_integration_test tests/step175_integration_test.cpp)
target_include_directories(step175_integration_test PRIVATE src)
target_link_libraries(step175_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step176_test tests/step176_test.cpp)
target_include_directories(step176_test PRIVATE src)

add_executable(step176_integration_test tests/step176_integration_test.cpp)
target_include_directories(step176_integration_test PRIVATE src)
target_link_libraries(step176_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step177_test tests/step177_test.cpp)
target_include_directories(step177_test PRIVATE src)

add_executable(step177_integration_test tests/step177_integration_test.cpp)
target_include_directories(step177_integration_test PRIVATE src)
target_link_libraries(step177_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step178_test tests/step178_test.cpp)
target_include_directories(step178_test PRIVATE src)

add_executable(step178_integration_test tests/step178_integration_test.cpp)
target_include_directories(step178_integration_test PRIVATE src)
target_link_libraries(step178_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step179_test tests/step179_test.cpp)
target_include_directories(step179_test PRIVATE src)

add_executable(step179_integration_test tests/step179_integration_test.cpp)
target_include_directories(step179_integration_test PRIVATE src)
target_link_libraries(step179_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step180_test tests/step180_test.cpp)
target_include_directories(step180_test PRIVATE src)

add_executable(step180_integration_test tests/step180_integration_test.cpp)
target_include_directories(step180_integration_test PRIVATE src)
target_link_libraries(step180_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step181_test tests/step181_test.cpp)
target_include_directories(step181_test PRIVATE src)

add_executable(step181_integration_test tests/step181_integration_test.cpp)
target_include_directories(step181_integration_test PRIVATE src)
target_link_libraries(step181_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step182_test tests/step182_test.cpp)
target_include_directories(step182_test PRIVATE src)

add_executable(step182_integration_test tests/step182_integration_test.cpp)
target_include_directories(step182_integration_test PRIVATE src)
target_link_libraries(step182_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step183_test tests/step183_test.cpp)
target_include_directories(step183_test PRIVATE src)

add_executable(step183_integration_test tests/step183_integration_test.cpp)
target_include_directories(step183_integration_test PRIVATE src)
target_link_libraries(step183_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step184_test tests/step184_test.cpp)
target_include_directories(step184_test PRIVATE src)

add_executable(step184_integration_test tests/step184_integration_test.cpp)
target_include_directories(step184_integration_test PRIVATE src)
target_link_libraries(step184_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step185_test tests/step185_test.cpp)
target_include_directories(step185_test PRIVATE src)

add_executable(step185_integration_test tests/step185_integration_test.cpp)
target_include_directories(step185_integration_test PRIVATE src)
target_link_libraries(step185_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step186_test tests/step186_test.cpp)
target_include_directories(step186_test PRIVATE src)

add_executable(step186_integration_test tests/step186_integration_test.cpp)
target_include_directories(step186_integration_test PRIVATE src)
target_link_libraries(step186_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step187_test tests/step187_test.cpp)
target_include_directories(step187_test PRIVATE src)

add_executable(step187_integration_test tests/step187_integration_test.cpp)
target_include_directories(step187_integration_test PRIVATE src)
target_link_libraries(step187_integration_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step188_test tests/step188_test.cpp)
target_include_directories(step188_test PRIVATE src)

add_executable(step189_test tests/step189_test.cpp)
target_include_directories(step189_test PRIVATE src)

add_executable(step190_test tests/step190_test.cpp)
target_include_directories(step190_test PRIVATE src)
target_link_libraries(step190_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step191_test tests/step191_test.cpp)
target_include_directories(step191_test PRIVATE src)

add_executable(step192_test tests/step192_test.cpp)
target_include_directories(step192_test PRIVATE src)

add_executable(step193_test tests/step193_test.cpp)
target_include_directories(step193_test PRIVATE src)

add_executable(step194_test tests/step194_test.cpp)
target_include_directories(step194_test PRIVATE src)

add_executable(step195_test tests/step195_test.cpp)
target_include_directories(step195_test PRIVATE src)

add_executable(step196_test tests/step196_test.cpp)
target_include_directories(step196_test PRIVATE src)

add_executable(step197_test tests/step197_test.cpp)
target_include_directories(step197_test PRIVATE src)

add_executable(step198_test tests/step198_test.cpp)
target_include_directories(step198_test PRIVATE src)

add_executable(step199_test tests/step199_test.cpp)
target_include_directories(step199_test PRIVATE src)

add_executable(step200_test tests/step200_test.cpp)
target_include_directories(step200_test PRIVATE src)

add_executable(step201_test tests/step201_test.cpp)
target_include_directories(step201_test PRIVATE src)

add_executable(step220_test tests/step220_test.cpp)
target_include_directories(step220_test PRIVATE src)

add_executable(step221_test tests/step221_test.cpp)
target_include_directories(step221_test PRIVATE src)

add_executable(step222_test tests/step222_test.cpp)
target_include_directories(step222_test PRIVATE src)

add_executable(step223_test tests/step223_test.cpp)
target_include_directories(step223_test PRIVATE src)

add_executable(step224_test tests/step224_test.cpp)
target_include_directories(step224_test PRIVATE src)
target_link_libraries(step224_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step225_test tests/step225_test.cpp)
target_include_directories(step225_test PRIVATE src)

add_executable(step226_test tests/step226_test.cpp)
target_include_directories(step226_test PRIVATE src)
add_executable(step227_test tests/step227_test.cpp)
target_include_directories(step227_test PRIVATE src)
add_executable(step228_test tests/step228_test.cpp)
target_include_directories(step228_test PRIVATE src)
target_link_libraries(step228_test PRIVATE nlohmann_json::nlohmann_json)
add_executable(step229_test tests/step229_test.cpp)
target_include_directories(step229_test PRIVATE src)
target_link_libraries(step229_test PRIVATE nlohmann_json::nlohmann_json)
add_executable(step230_test tests/step230_test.cpp)
target_include_directories(step230_test PRIVATE src)
target_link_libraries(step230_test PRIVATE nlohmann_json::nlohmann_json)
add_executable(step231_test tests/step231_test.cpp)
target_include_directories(step231_test PRIVATE src)
target_link_libraries(step231_test PRIVATE nlohmann_json::nlohmann_json)
add_executable(step232_test tests/step232_test.cpp)
target_include_directories(step232_test PRIVATE src)
target_link_libraries(step232_test PRIVATE nlohmann_json::nlohmann_json)
add_executable(step233_test tests/step233_test.cpp)
target_include_directories(step233_test PRIVATE src)
target_link_libraries(step233_test PRIVATE nlohmann_json::nlohmann_json)
add_executable(step234_test tests/step234_test.cpp)
target_include_directories(step234_test PRIVATE src)
target_link_libraries(step234_test PRIVATE nlohmann_json::nlohmann_json)
add_executable(step244_test tests/step244_test.cpp)
target_include_directories(step244_test PRIVATE src)
target_link_libraries(step244_test PRIVATE nlohmann_json::nlohmann_json)
add_executable(step213_test tests/step213_test.cpp)
target_include_directories(step213_test PRIVATE src)
target_link_libraries(step213_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step219_test tests/step219_test.cpp)
target_include_directories(step219_test PRIVATE src)
target_link_libraries(step219_test PRIVATE
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

add_executable(step206_test tests/step206_test.cpp)
target_include_directories(step206_test PRIVATE src)
target_link_libraries(step206_test PRIVATE
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

if(UNIX AND NOT APPLE)
    # Force system SDL2 on Linux. The vcpkg toolchain overrides PKG_CONFIG_PATH,
    # causing pkg_check_modules to find vcpkg's headless (offscreen-only) SDL2.
    # We bypass that by pointing pkg-config directly at the system .pc files.
    find_package(PkgConfig REQUIRED)
    set(_saved_pkg_config_path "$ENV{PKG_CONFIG_PATH}")
    set(ENV{PKG_CONFIG_PATH} "/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig")
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    set(ENV{PKG_CONFIG_PATH} "${_saved_pkg_config_path}")
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    target_link_libraries(SDL2::SDL2 INTERFACE PkgConfig::SDL2)
else()
    find_package(SDL2 CONFIG REQUIRED)
endif()
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
target_compile_definitions(whetstone_editor PRIVATE WHETSTONE_ENABLE_ORG=1)
if(IMGUI_SDL2_BACKEND)
    get_filename_component(_sdl2_backend_dir "${IMGUI_SDL2_BACKEND}" DIRECTORY)
    target_include_directories(whetstone_editor PRIVATE "${_sdl2_backend_dir}")
endif()
target_link_libraries(whetstone_editor PRIVATE
    SDL2::SDL2
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
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
    tree_sitter_go
    tree_sitter_org)
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

add_executable(whetstone_eval src/eval_main.cpp)
target_include_directories(whetstone_eval PRIVATE src)
target_link_libraries(whetstone_eval PRIVATE nlohmann_json::nlohmann_json)

add_executable(whetstone_pipeline src/pipeline_main.cpp)
target_include_directories(whetstone_pipeline PRIVATE src)
target_link_libraries(whetstone_pipeline PRIVATE nlohmann_json::nlohmann_json)

# Step 245: Headless EditorState test (no SDL/ImGui dependency)
add_executable(step245_test tests/step245_test.cpp)
target_include_directories(step245_test PRIVATE src)
target_link_libraries(step245_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_cpp
    tree_sitter_elisp
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_java
    tree_sitter_rust
    tree_sitter_go
    tree_sitter_org)

# Step 246: Standalone MCP server (no SDL/ImGui)
add_executable(whetstone_mcp src/mcp_main.cpp)
target_include_directories(whetstone_mcp PRIVATE src)
target_link_libraries(whetstone_mcp PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

add_executable(step246_test tests/step246_test.cpp)
target_include_directories(step246_test PRIVATE src)
target_link_libraries(step246_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 247: File operation tools for MCP
add_executable(step247_test tests/step247_test.cpp)
target_include_directories(step247_test PRIVATE src)
target_link_libraries(step247_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 248: Compact AST response format
add_executable(step248_test tests/step248_test.cpp)
target_include_directories(step248_test PRIVATE src)
target_link_libraries(step248_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 249: MCP server integration tests
add_executable(step249_test tests/step249_test.cpp)
target_include_directories(step249_test PRIVATE src)
target_link_libraries(step249_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 250: Structured diagnostic format
add_executable(step250_test tests/step250_test.cpp)
target_include_directories(step250_test PRIVATE src)
target_link_libraries(step250_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 251: Quick-fix actions as RPC mutations
add_executable(step251_test tests/step251_test.cpp)
target_include_directories(step251_test PRIVATE src)
target_link_libraries(step251_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 252: Diagnostic delta streaming
add_executable(step252_test tests/step252_test.cpp)
target_include_directories(step252_test PRIVATE src)
target_link_libraries(step252_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 253: Diagnostic and delta integration tests
add_executable(step253_test tests/step253_test.cpp)
target_include_directories(step253_test PRIVATE src)
target_link_libraries(step253_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 254: Response budget system
add_executable(step254_test tests/step254_test.cpp)
target_include_directories(step254_test PRIVATE src)
target_link_libraries(step254_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

add_executable(step255_test tests/step255_test.cpp)
target_include_directories(step255_test PRIVATE src)
target_link_libraries(step255_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

add_executable(step256_test tests/step256_test.cpp)
target_include_directories(step256_test PRIVATE src)
target_link_libraries(step256_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 257: Token efficiency tests + benchmarks
add_executable(step257_test tests/step257_test.cpp)
target_include_directories(step257_test PRIVATE src)
target_link_libraries(step257_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 258: Project model and workspace indexing
add_executable(step258_test tests/step258_test.cpp)
target_include_directories(step258_test PRIVATE src)
target_link_libraries(step258_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 259: Cross-file symbol resolution
add_executable(step259_test tests/step259_test.cpp)
target_include_directories(step259_test PRIVATE src)
target_link_libraries(step259_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 260: Project-wide diagnostics
add_executable(step260_test tests/step260_test.cpp)
target_include_directories(step260_test PRIVATE src)
target_link_libraries(step260_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 261: Project-wide search and refactor
add_executable(step261_test tests/step261_test.cpp)
target_include_directories(step261_test PRIVATE src)
target_link_libraries(step261_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 262: Phase 9d multi-file project integration tests
add_executable(step262_test tests/step262_test.cpp)
target_include_directories(step262_test PRIVATE src)
target_link_libraries(step262_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 263: Save buffer to disk
add_executable(step263_test tests/step263_test.cpp)
target_include_directories(step263_test PRIVATE src)
target_link_libraries(step263_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 264: Undo/redo for headless mode
add_executable(step264_test tests/step264_test.cpp)
target_include_directories(step264_test PRIVATE src)
target_link_libraries(step264_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

add_executable(step265_test tests/step265_test.cpp)
target_include_directories(step265_test PRIVATE src)
target_link_libraries(step265_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

add_executable(step266_test tests/step266_test.cpp)
target_include_directories(step266_test PRIVATE src)
target_link_libraries(step266_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 267: Sidecar AST persistence
add_executable(step267_test tests/step267_test.cpp)
target_include_directories(step267_test PRIVATE src)
target_link_libraries(step267_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 270: Annotation prompt templates
add_executable(step270_test tests/step270_test.cpp)
target_include_directories(step270_test PRIVATE src)
target_link_libraries(step270_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 269: Annotation RPC methods
add_executable(step269_test tests/step269_test.cpp)
target_include_directories(step269_test PRIVATE src)
target_link_libraries(step269_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 268: Phase 10a integration tests
add_executable(step268_test tests/step268_test.cpp)
target_include_directories(step268_test PRIVATE src)
target_link_libraries(step268_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 271: Phase 10b integration tests
add_executable(step271_test tests/step271_test.cpp)
target_include_directories(step271_test PRIVATE src)
target_link_libraries(step271_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 272: Type System — Layout & Constraints
add_executable(step272_test tests/step272_test.cpp)
target_include_directories(step272_test PRIVATE src)
target_link_libraries(step272_test PRIVATE nlohmann_json::nlohmann_json)

# Step 273: Type System — Identity & Mutability
add_executable(step273_test tests/step273_test.cpp)
target_include_directories(step273_test PRIVATE src)
target_link_libraries(step273_test PRIVATE nlohmann_json::nlohmann_json)

# Step 274: Concurrency — Primitives & Memory Model
add_executable(step274_test tests/step274_test.cpp)
target_include_directories(step274_test PRIVATE src)
target_link_libraries(step274_test PRIVATE nlohmann_json::nlohmann_json)

# Step 275: Async, Parallelism & Error Handling
add_executable(step275_test tests/step275_test.cpp)
target_include_directories(step275_test PRIVATE src)
target_link_libraries(step275_test PRIVATE nlohmann_json::nlohmann_json)

# Step 276: Scope & Namespace Annotations
add_executable(step276_test tests/step276_test.cpp)
target_include_directories(step276_test PRIVATE src)
target_link_libraries(step276_test PRIVATE nlohmann_json::nlohmann_json)

# Step 277: Phase 10c Integration Tests
add_executable(step277_test tests/step277_test.cpp)
target_include_directories(step277_test PRIVATE src)
target_link_libraries(step277_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 278: Shim & Escape Hatch Annotations
add_executable(step278_test tests/step278_test.cpp)
target_include_directories(step278_test PRIVATE src)
target_link_libraries(step278_test PRIVATE nlohmann_json::nlohmann_json)

# Step 279: Platform & Provenance Annotations
add_executable(step279_test tests/step279_test.cpp)
target_include_directories(step279_test PRIVATE src)
target_link_libraries(step279_test PRIVATE nlohmann_json::nlohmann_json)

# Step 280: Optimization Annotation Completion
add_executable(step280_test tests/step280_test.cpp)
target_include_directories(step280_test PRIVATE src)
target_link_libraries(step280_test PRIVATE nlohmann_json::nlohmann_json)

# Step 281: Meta-Programming Annotations
add_executable(step281_test tests/step281_test.cpp)
target_include_directories(step281_test PRIVATE src)
target_link_libraries(step281_test PRIVATE nlohmann_json::nlohmann_json)

# Step 282: Strategy & Policy Annotations
add_executable(step282_test tests/step282_test.cpp)
target_include_directories(step282_test PRIVATE src)
target_link_libraries(step282_test PRIVATE nlohmann_json::nlohmann_json)

# Step 283: Phase 10d Integration Tests
add_executable(step283_test tests/step283_test.cpp)
target_include_directories(step283_test PRIVATE src)
target_link_libraries(step283_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 284: EnvironmentSpec schema & AST node
add_executable(step284_test tests/step284_test.cpp)
target_include_directories(step284_test PRIVATE src)
target_link_libraries(step284_test PRIVATE nlohmann_json::nlohmann_json)

# Step 285: Capability vocabulary & validation
add_executable(step285_test tests/step285_test.cpp)
target_include_directories(step285_test PRIVATE src)
target_link_libraries(step285_test PRIVATE nlohmann_json::nlohmann_json)

# Step 286: Environment-aware pipeline hooks
add_executable(step286_test tests/step286_test.cpp)
target_include_directories(step286_test PRIVATE src)
target_link_libraries(step286_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 287: Environment-aware lowering decisions
add_executable(step287_test tests/step287_test.cpp)
target_include_directories(step287_test PRIVATE src)
target_link_libraries(step287_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Step 288: Host boundary AST nodes
add_executable(step288_test tests/step288_test.cpp)
target_include_directories(step288_test PRIVATE src)
target_link_libraries(step288_test PRIVATE nlohmann_json::nlohmann_json)

# Step 289: Phase 10e integration tests
add_executable(step289_test tests/step289_test.cpp)
target_include_directories(step289_test PRIVATE src)
target_link_libraries(step289_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go
    tree_sitter_org)

# Phase 11a: Semanno Format + Annotation Codegen (Steps 290-296)
foreach(STEP 290 291 292 293 294 295 296)
    add_executable(step${STEP}_test tests/step${STEP}_test.cpp)
    target_include_directories(step${STEP}_test PRIVATE src)
    target_link_libraries(step${STEP}_test PRIVATE
        nlohmann_json::nlohmann_json
        unofficial::tree-sitter::tree-sitter
        tree_sitter_python tree_sitter_cpp tree_sitter_elisp
        tree_sitter_javascript tree_sitter_typescript
        tree_sitter_java tree_sitter_rust tree_sitter_go
        tree_sitter_org)
endforeach()

# Phase 11b: Validation & Conflict Completion (Steps 297-301)
foreach(STEP 297 298 299 300 301)
    add_executable(step${STEP}_test tests/step${STEP}_test.cpp)
    target_include_directories(step${STEP}_test PRIVATE src)
    target_link_libraries(step${STEP}_test PRIVATE
        nlohmann_json::nlohmann_json
        unofficial::tree-sitter::tree-sitter
        tree_sitter_python tree_sitter_cpp tree_sitter_elisp
        tree_sitter_javascript tree_sitter_typescript
        tree_sitter_java tree_sitter_rust tree_sitter_go
        tree_sitter_org)
endforeach()

# Phase 11c: Parser Deepening (Steps 302-308)
add_executable(step302_test tests/step302_test.cpp)
target_include_directories(step302_test PRIVATE src)
target_link_libraries(step302_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step303_test tests/step303_test.cpp)
target_include_directories(step303_test PRIVATE src)
target_link_libraries(step303_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step304_test tests/step304_test.cpp)
target_include_directories(step304_test PRIVATE src)
target_link_libraries(step304_test PRIVATE nlohmann_json::nlohmann_json)

# Step 305: Python + Java Parser Deepening
add_executable(step305_test tests/step305_test.cpp)
target_include_directories(step305_test PRIVATE src)
target_link_libraries(step305_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python
    tree_sitter_java)

# Step 306: TypeScript/JavaScript + Rust Parser Deepening
add_executable(step306_test tests/step306_test.cpp)
target_include_directories(step306_test PRIVATE src)
target_link_libraries(step306_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_javascript
    tree_sitter_typescript
    tree_sitter_rust)

# Step 307: Go + C++ + Elisp Parser Deepening
add_executable(step307_test tests/step307_test.cpp)
target_include_directories(step307_test PRIVATE src)
target_link_libraries(step307_test PRIVATE
    unofficial::tree-sitter::tree-sitter
    tree_sitter_go
    tree_sitter_cpp
    tree_sitter_elisp)

# Step 308: Generator Updates + Phase 11c Integration Tests
add_executable(step308_test tests/step308_test.cpp)
target_include_directories(step308_test PRIVATE src)
target_link_libraries(step308_test PRIVATE nlohmann_json::nlohmann_json)

# Step 309: Kotlin Parser Tests
add_executable(step309_test tests/step309_test.cpp)
target_include_directories(step309_test PRIVATE src)
target_link_libraries(step309_test PRIVATE nlohmann_json::nlohmann_json)

# Step 310: Kotlin Generator Tests
add_executable(step310_test tests/step310_test.cpp)
target_include_directories(step310_test PRIVATE src)
target_link_libraries(step310_test PRIVATE nlohmann_json::nlohmann_json)

# Step 311: C# Parser Tests
add_executable(step311_test tests/step311_test.cpp)
target_include_directories(step311_test PRIVATE src)
target_link_libraries(step311_test PRIVATE nlohmann_json::nlohmann_json)

# Step 312: C# Generator Tests
add_executable(step312_test tests/step312_test.cpp)
target_include_directories(step312_test PRIVATE src)
target_link_libraries(step312_test PRIVATE nlohmann_json::nlohmann_json)

# Step 313: New Languages Integration Tests
add_executable(step313_test tests/step313_test.cpp)
target_include_directories(step313_test PRIVATE src)
target_link_libraries(step313_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Step 314: Generalized Annotation Inference Engine
add_executable(step314_test tests/step314_test.cpp)
target_include_directories(step314_test PRIVATE src)
target_link_libraries(step314_test PRIVATE nlohmann_json::nlohmann_json)

# Step 315: Routing Annotation Types (Subject 9)
add_executable(step315_test tests/step315_test.cpp)
target_include_directories(step315_test PRIVATE src)
target_link_libraries(step315_test PRIVATE nlohmann_json::nlohmann_json)

# Step 316: Skeleton AST Support
add_executable(step316_test tests/step316_test.cpp)
target_include_directories(step316_test PRIVATE src)
target_link_libraries(step316_test PRIVATE nlohmann_json::nlohmann_json)

# Step 317: Architect Tooling — RPC + MCP
add_executable(step317_test tests/step317_test.cpp)
target_include_directories(step317_test PRIVATE src)
target_link_libraries(step317_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step318_test tests/step318_test.cpp)
target_include_directories(step318_test PRIVATE src)
target_link_libraries(step318_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Step 320: WorkItem — Core Execution Model
add_executable(step320_test tests/step320_test.cpp)
target_include_directories(step320_test PRIVATE src)
target_link_libraries(step320_test PRIVATE nlohmann_json::nlohmann_json)

# Step 321: TaskQueue — Priority Queue with Dependencies
add_executable(step321_test tests/step321_test.cpp)
target_include_directories(step321_test PRIVATE src)
target_link_libraries(step321_test PRIVATE nlohmann_json::nlohmann_json)

# Step 322: WorkflowState — Project-Level Workflow Tracking
add_executable(step322_test tests/step322_test.cpp)
target_include_directories(step322_test PRIVATE src)
target_link_libraries(step322_test PRIVATE nlohmann_json::nlohmann_json)

# Step 323: Workflow Sidecar Persistence
add_executable(step323_test tests/step323_test.cpp)
target_include_directories(step323_test PRIVATE src)
target_link_libraries(step323_test PRIVATE nlohmann_json::nlohmann_json)

# Step 324: Workflow RPC + MCP Tools
add_executable(step324_test tests/step324_test.cpp)
target_include_directories(step324_test PRIVATE src)
target_link_libraries(step324_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Step 325: Phase 12a Integration Tests
add_executable(step325_test tests/step325_test.cpp)
target_include_directories(step325_test PRIVATE src)
target_link_libraries(step325_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Step 326: RoutingEngine — Annotation-to-Dispatch Logic
add_executable(step326_test tests/step326_test.cpp)
target_include_directories(step326_test PRIVATE src)
target_link_libraries(step326_test PRIVATE nlohmann_json::nlohmann_json)

# Step 327: WorkerRegistry — Worker Abstractions
add_executable(step327_test tests/step327_test.cpp)
target_include_directories(step327_test PRIVATE src)
target_link_libraries(step327_test PRIVATE nlohmann_json::nlohmann_json)

# Step 328: Context Assembly + Budget
add_executable(step328_test tests/step328_test.cpp)
target_include_directories(step328_test PRIVATE src)
target_link_libraries(step328_test PRIVATE nlohmann_json::nlohmann_json)

# Step 329: Routing RPC + MCP Tools
add_executable(step329_test tests/step329_test.cpp)
target_include_directories(step329_test PRIVATE src)
target_link_libraries(step329_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Step 330: Review Gates
add_executable(step330_test tests/step330_test.cpp)
target_include_directories(step330_test PRIVATE src)
target_link_libraries(step330_test PRIVATE nlohmann_json::nlohmann_json)

# Step 331: Phase 12b Integration Tests
add_executable(step331_test tests/step331_test.cpp)
target_include_directories(step331_test PRIVATE src)
target_link_libraries(step331_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Step 332: Multiple Inheritance in ClassDeclaration
add_executable(step332_test tests/step332_test.cpp)
target_include_directories(step332_test PRIVATE src)
target_link_libraries(step332_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step333_test tests/step333_test.cpp)
target_include_directories(step333_test PRIVATE src)
target_link_libraries(step333_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step334_test tests/step334_test.cpp)
target_include_directories(step334_test PRIVATE src)
target_link_libraries(step334_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step335_test tests/step335_test.cpp)
target_include_directories(step335_test PRIVATE src)
target_link_libraries(step335_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step336_test tests/step336_test.cpp)
target_include_directories(step336_test PRIVATE src)
target_link_libraries(step336_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step337_test tests/step337_test.cpp)
target_include_directories(step337_test PRIVATE src)
target_link_libraries(step337_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step338_test tests/step338_test.cpp)
target_include_directories(step338_test PRIVATE src)
target_link_libraries(step338_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step339_test tests/step339_test.cpp)
target_include_directories(step339_test PRIVATE src)
target_link_libraries(step339_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step340_test tests/step340_test.cpp)
target_include_directories(step340_test PRIVATE src)
target_link_libraries(step340_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step341_test tests/step341_test.cpp)
target_include_directories(step341_test PRIVATE src)
target_link_libraries(step341_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step342_test tests/step342_test.cpp)
target_include_directories(step342_test PRIVATE src)
target_link_libraries(step342_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step343_test tests/step343_test.cpp)
target_include_directories(step343_test PRIVATE src)
target_link_libraries(step343_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step344_test tests/step344_test.cpp)
target_include_directories(step344_test PRIVATE src)
target_link_libraries(step344_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step345_test tests/step345_test.cpp)
target_include_directories(step345_test PRIVATE src)
target_link_libraries(step345_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step346_test tests/step346_test.cpp)
target_include_directories(step346_test PRIVATE src)
target_link_libraries(step346_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step347_test tests/step347_test.cpp)
target_include_directories(step347_test PRIVATE src)
target_link_libraries(step347_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step348_test tests/step348_test.cpp)
target_include_directories(step348_test PRIVATE src)
target_link_libraries(step348_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step349_test tests/step349_test.cpp)
target_include_directories(step349_test PRIVATE src)
target_link_libraries(step349_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step350_test tests/step350_test.cpp)
target_include_directories(step350_test PRIVATE src)
target_link_libraries(step350_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step351_test tests/step351_test.cpp)
target_include_directories(step351_test PRIVATE src)
target_link_libraries(step351_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step352_test tests/step352_test.cpp)
target_include_directories(step352_test PRIVATE src)
target_link_libraries(step352_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step353_test tests/step353_test.cpp)
target_include_directories(step353_test PRIVATE src)
target_link_libraries(step353_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step354_test tests/step354_test.cpp)
target_include_directories(step354_test PRIVATE src)
target_link_libraries(step354_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step355_test tests/step355_test.cpp)
target_include_directories(step355_test PRIVATE src)
target_link_libraries(step355_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step356_test tests/step356_test.cpp)
target_include_directories(step356_test PRIVATE src)
target_link_libraries(step356_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step357_test tests/step357_test.cpp)
target_include_directories(step357_test PRIVATE src)
target_link_libraries(step357_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step358_test tests/step358_test.cpp)
target_include_directories(step358_test PRIVATE src)
target_link_libraries(step358_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step359_test tests/step359_test.cpp)
target_include_directories(step359_test PRIVATE src)
target_link_libraries(step359_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step360_test tests/step360_test.cpp)
target_include_directories(step360_test PRIVATE src)
target_link_libraries(step360_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step361_test tests/step361_test.cpp)
target_include_directories(step361_test PRIVATE src)
target_link_libraries(step361_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step362_test tests/step362_test.cpp)
target_include_directories(step362_test PRIVATE src)
target_link_libraries(step362_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step363_test tests/step363_test.cpp)
target_include_directories(step363_test PRIVATE src)
target_link_libraries(step363_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step364_test tests/step364_test.cpp)
target_include_directories(step364_test PRIVATE src)
target_link_libraries(step364_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step365_test tests/step365_test.cpp)
target_include_directories(step365_test PRIVATE src)
target_link_libraries(step365_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step366_test tests/step366_test.cpp)
target_include_directories(step366_test PRIVATE src)
target_link_libraries(step366_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step367_test tests/step367_test.cpp)
target_include_directories(step367_test PRIVATE src)
target_link_libraries(step367_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step368_test tests/step368_test.cpp)
target_include_directories(step368_test PRIVATE src)
target_link_libraries(step368_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step369_test tests/step369_test.cpp)
target_include_directories(step369_test PRIVATE src)
target_link_libraries(step369_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step370_test tests/step370_test.cpp)
target_include_directories(step370_test PRIVATE src)
target_link_libraries(step370_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step371_test tests/step371_test.cpp)
target_include_directories(step371_test PRIVATE src)
target_link_libraries(step371_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step372_test tests/step372_test.cpp)
target_include_directories(step372_test PRIVATE src)
target_link_libraries(step372_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step373_test tests/step373_test.cpp)
target_include_directories(step373_test PRIVATE src)
target_link_libraries(step373_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step374_test tests/step374_test.cpp)
target_include_directories(step374_test PRIVATE src)
target_link_libraries(step374_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step375_test tests/step375_test.cpp)
target_include_directories(step375_test PRIVATE src)
target_link_libraries(step375_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step376_test tests/step376_test.cpp)
target_include_directories(step376_test PRIVATE src)
target_link_libraries(step376_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step377_test tests/step377_test.cpp)
target_include_directories(step377_test PRIVATE src)
target_link_libraries(step377_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step378_test tests/step378_test.cpp)
target_include_directories(step378_test PRIVATE src)
target_link_libraries(step378_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step379_test tests/step379_test.cpp)
target_include_directories(step379_test PRIVATE src)
target_link_libraries(step379_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step380_test tests/step380_test.cpp)
target_include_directories(step380_test PRIVATE src)
target_link_libraries(step380_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step381_test tests/step381_test.cpp)
target_include_directories(step381_test PRIVATE src)
target_link_libraries(step381_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step382_test tests/step382_test.cpp)
target_include_directories(step382_test PRIVATE src)
target_link_libraries(step382_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step383_test tests/step383_test.cpp)
target_include_directories(step383_test PRIVATE src)
target_link_libraries(step383_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step384_test tests/step384_test.cpp)
target_include_directories(step384_test PRIVATE src)
target_link_libraries(step384_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step385_test tests/step385_test.cpp)
target_include_directories(step385_test PRIVATE src)
target_link_libraries(step385_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step386_test tests/step386_test.cpp)
target_include_directories(step386_test PRIVATE src)
target_link_libraries(step386_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step387_test tests/step387_test.cpp)
target_include_directories(step387_test PRIVATE src)
target_link_libraries(step387_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step388_test tests/step388_test.cpp)
target_include_directories(step388_test PRIVATE src)
target_link_libraries(step388_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step389_test tests/step389_test.cpp)
target_include_directories(step389_test PRIVATE src)
target_link_libraries(step389_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step390_test tests/step390_test.cpp)
target_include_directories(step390_test PRIVATE src)
target_link_libraries(step390_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step391_test tests/step391_test.cpp)
target_include_directories(step391_test PRIVATE src)
target_link_libraries(step391_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step392_test tests/step392_test.cpp)
target_include_directories(step392_test PRIVATE src)
target_link_libraries(step392_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step393_test tests/step393_test.cpp)
target_include_directories(step393_test PRIVATE src)
target_link_libraries(step393_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step394_test tests/step394_test.cpp)
target_include_directories(step394_test PRIVATE src)
target_link_libraries(step394_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step395_test tests/step395_test.cpp)
target_include_directories(step395_test PRIVATE src)
target_link_libraries(step395_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step396_test tests/step396_test.cpp)
target_include_directories(step396_test PRIVATE src)
target_link_libraries(step396_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step397_test tests/step397_test.cpp)
target_include_directories(step397_test PRIVATE src)
target_link_libraries(step397_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step398_test tests/step398_test.cpp)
target_include_directories(step398_test PRIVATE src)
target_link_libraries(step398_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step399_test tests/step399_test.cpp)
target_include_directories(step399_test PRIVATE src)
target_link_libraries(step399_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step400_test tests/step400_test.cpp)
target_include_directories(step400_test PRIVATE src)
target_link_libraries(step400_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step401_test tests/step401_test.cpp)
target_include_directories(step401_test PRIVATE src)
target_link_libraries(step401_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step402_test tests/step402_test.cpp)
target_include_directories(step402_test PRIVATE src)
target_link_libraries(step402_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step403_test tests/step403_test.cpp)
target_include_directories(step403_test PRIVATE src)
target_link_libraries(step403_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step404_test tests/step404_test.cpp)
target_include_directories(step404_test PRIVATE src)
target_link_libraries(step404_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step405_test tests/step405_test.cpp)
target_include_directories(step405_test PRIVATE src)
target_link_libraries(step405_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step406_test tests/step406_test.cpp)
target_include_directories(step406_test PRIVATE src)
target_link_libraries(step406_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step407_test tests/step407_test.cpp)
target_include_directories(step407_test PRIVATE src)
target_link_libraries(step407_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step408_test tests/step408_test.cpp)
target_include_directories(step408_test PRIVATE src)
target_link_libraries(step408_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step409_test tests/step409_test.cpp)
target_include_directories(step409_test PRIVATE src)
target_link_libraries(step409_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step410_test tests/step410_test.cpp)
target_include_directories(step410_test PRIVATE src)
target_link_libraries(step410_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step411_test tests/step411_test.cpp)
target_include_directories(step411_test PRIVATE src)
target_link_libraries(step411_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step412_test tests/step412_test.cpp)
target_include_directories(step412_test PRIVATE src)
target_link_libraries(step412_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step413_test tests/step413_test.cpp)
target_include_directories(step413_test PRIVATE src)
target_link_libraries(step413_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step414_test tests/step414_test.cpp)
target_include_directories(step414_test PRIVATE src)
target_link_libraries(step414_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step415_test tests/step415_test.cpp)
target_include_directories(step415_test PRIVATE src)
target_link_libraries(step415_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step416_test tests/step416_test.cpp)
target_include_directories(step416_test PRIVATE src)
target_link_libraries(step416_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step417_test tests/step417_test.cpp)
target_include_directories(step417_test PRIVATE src)
target_link_libraries(step417_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step418_test tests/step418_test.cpp)
target_include_directories(step418_test PRIVATE src)
target_link_libraries(step418_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step419_test tests/step419_test.cpp)
target_include_directories(step419_test PRIVATE src)
target_link_libraries(step419_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step420_test tests/step420_test.cpp)
target_include_directories(step420_test PRIVATE src)
target_link_libraries(step420_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step421_test tests/step421_test.cpp)
target_include_directories(step421_test PRIVATE src)
target_link_libraries(step421_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step422_test tests/step422_test.cpp)
target_include_directories(step422_test PRIVATE src)
target_link_libraries(step422_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step423_test tests/step423_test.cpp)
target_include_directories(step423_test PRIVATE src)
target_link_libraries(step423_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step424_test tests/step424_test.cpp)
target_include_directories(step424_test PRIVATE src)
target_link_libraries(step424_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step425_test tests/step425_test.cpp)
target_include_directories(step425_test PRIVATE src)
target_link_libraries(step425_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step426_test tests/step426_test.cpp)
target_include_directories(step426_test PRIVATE src)
target_link_libraries(step426_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step427_test tests/step427_test.cpp)
target_include_directories(step427_test PRIVATE src)
target_link_libraries(step427_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step428_test tests/step428_test.cpp)
target_include_directories(step428_test PRIVATE src)
target_link_libraries(step428_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step429_test tests/step429_test.cpp)
target_include_directories(step429_test PRIVATE src)
target_link_libraries(step429_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step430_test tests/step430_test.cpp)
target_include_directories(step430_test PRIVATE src)
target_link_libraries(step430_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step431_test tests/step431_test.cpp)
target_include_directories(step431_test PRIVATE src)
target_link_libraries(step431_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step432_test tests/step432_test.cpp)
target_include_directories(step432_test PRIVATE src)
target_link_libraries(step432_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step433_test tests/step433_test.cpp)
target_include_directories(step433_test PRIVATE src)
target_link_libraries(step433_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step434_test tests/step434_test.cpp)
target_include_directories(step434_test PRIVATE src)
target_link_libraries(step434_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step435_test tests/step435_test.cpp)
target_include_directories(step435_test PRIVATE src)
target_link_libraries(step435_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step436_test tests/step436_test.cpp)
target_include_directories(step436_test PRIVATE src)
target_link_libraries(step436_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step437_test tests/step437_test.cpp)
target_include_directories(step437_test PRIVATE src)
target_link_libraries(step437_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step438_test tests/step438_test.cpp)
target_include_directories(step438_test PRIVATE src)
target_link_libraries(step438_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step439_test tests/step439_test.cpp)
target_include_directories(step439_test PRIVATE src)
target_link_libraries(step439_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step440_test tests/step440_test.cpp)
target_include_directories(step440_test PRIVATE src)
target_link_libraries(step440_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step441_test tests/step441_test.cpp)
target_include_directories(step441_test PRIVATE src)
target_link_libraries(step441_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step442_test tests/step442_test.cpp)
target_include_directories(step442_test PRIVATE src)
target_link_libraries(step442_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step443_test tests/step443_test.cpp)
target_include_directories(step443_test PRIVATE src)
target_link_libraries(step443_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step444_test tests/step444_test.cpp)
target_include_directories(step444_test PRIVATE src)
target_link_libraries(step444_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step445_test tests/step445_test.cpp)
target_include_directories(step445_test PRIVATE src)
target_link_libraries(step445_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step446_test tests/step446_test.cpp)
target_include_directories(step446_test PRIVATE src)
target_link_libraries(step446_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step447_test tests/step447_test.cpp)
target_include_directories(step447_test PRIVATE src)
target_link_libraries(step447_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step448_test tests/step448_test.cpp)
target_include_directories(step448_test PRIVATE src)
target_link_libraries(step448_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step449_test tests/step449_test.cpp)
target_include_directories(step449_test PRIVATE src)
target_link_libraries(step449_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step450_test tests/step450_test.cpp)
target_include_directories(step450_test PRIVATE src)
target_link_libraries(step450_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step451_test tests/step451_test.cpp)
target_include_directories(step451_test PRIVATE src)
target_link_libraries(step451_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step452_test tests/step452_test.cpp)
target_include_directories(step452_test PRIVATE src)
target_link_libraries(step452_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step453_test tests/step453_test.cpp)
target_include_directories(step453_test PRIVATE src)
target_link_libraries(step453_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step454_test tests/step454_test.cpp)
target_include_directories(step454_test PRIVATE src)
target_link_libraries(step454_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step455_test tests/step455_test.cpp)
target_include_directories(step455_test PRIVATE src)
target_link_libraries(step455_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step456_test tests/step456_test.cpp)
target_include_directories(step456_test PRIVATE src)
target_link_libraries(step456_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step457_test tests/step457_test.cpp)
target_include_directories(step457_test PRIVATE src)
target_link_libraries(step457_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step458_test tests/step458_test.cpp)
target_include_directories(step458_test PRIVATE src)
target_link_libraries(step458_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step459_test tests/step459_test.cpp)
target_include_directories(step459_test PRIVATE src)
target_link_libraries(step459_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step460_test tests/step460_test.cpp)
target_include_directories(step460_test PRIVATE src)
target_link_libraries(step460_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step461_test tests/step461_test.cpp)
target_include_directories(step461_test PRIVATE src)
target_link_libraries(step461_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step462_test tests/step462_test.cpp)
target_include_directories(step462_test PRIVATE src)
target_link_libraries(step462_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step463_test tests/step463_test.cpp)
target_include_directories(step463_test PRIVATE src)
target_link_libraries(step463_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step464_test tests/step464_test.cpp)
target_include_directories(step464_test PRIVATE src)
target_link_libraries(step464_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step465_test tests/step465_test.cpp)
target_include_directories(step465_test PRIVATE src)
target_link_libraries(step465_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step466_test tests/step466_test.cpp)
target_include_directories(step466_test PRIVATE src)
target_link_libraries(step466_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step467_test tests/step467_test.cpp)
target_include_directories(step467_test PRIVATE src)
target_link_libraries(step467_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step468_test tests/step468_test.cpp)
target_include_directories(step468_test PRIVATE src)
target_link_libraries(step468_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step469_test tests/step469_test.cpp)
target_include_directories(step469_test PRIVATE src)
target_link_libraries(step469_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step470_test tests/step470_test.cpp)
target_include_directories(step470_test PRIVATE src)
target_link_libraries(step470_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step471_test tests/step471_test.cpp)
target_include_directories(step471_test PRIVATE src)
target_link_libraries(step471_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step472_test tests/step472_test.cpp)
target_include_directories(step472_test PRIVATE src)
target_link_libraries(step472_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step473_test tests/step473_test.cpp)
target_include_directories(step473_test PRIVATE src)
target_link_libraries(step473_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step474_test tests/step474_test.cpp)
target_include_directories(step474_test PRIVATE src)
target_link_libraries(step474_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step475_test tests/step475_test.cpp)
target_include_directories(step475_test PRIVATE src)
target_link_libraries(step475_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step476_test tests/step476_test.cpp)
target_include_directories(step476_test PRIVATE src)
target_link_libraries(step476_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step477_test tests/step477_test.cpp)
target_include_directories(step477_test PRIVATE src)
target_link_libraries(step477_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step478_test tests/step478_test.cpp)
target_include_directories(step478_test PRIVATE src)
target_link_libraries(step478_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step479_test tests/step479_test.cpp)
target_include_directories(step479_test PRIVATE src)
target_link_libraries(step479_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step480_test tests/step480_test.cpp)
target_include_directories(step480_test PRIVATE src)
target_link_libraries(step480_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step481_test tests/step481_test.cpp)
target_include_directories(step481_test PRIVATE src)
target_link_libraries(step481_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step482_test tests/step482_test.cpp)
target_include_directories(step482_test PRIVATE src)
target_link_libraries(step482_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step483_test tests/step483_test.cpp)
target_include_directories(step483_test PRIVATE src)
target_link_libraries(step483_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step484_test tests/step484_test.cpp)
target_include_directories(step484_test PRIVATE src)
target_link_libraries(step484_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step485_test tests/step485_test.cpp)
target_include_directories(step485_test PRIVATE src)
target_link_libraries(step485_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step486_test tests/step486_test.cpp)
target_include_directories(step486_test PRIVATE src)
target_link_libraries(step486_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step487_test tests/step487_test.cpp)
target_include_directories(step487_test PRIVATE src)
target_link_libraries(step487_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step488_test tests/step488_test.cpp)
target_include_directories(step488_test PRIVATE src)
target_link_libraries(step488_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step489_test tests/step489_test.cpp)
target_include_directories(step489_test PRIVATE src)
target_link_libraries(step489_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step490_test tests/step490_test.cpp)
target_include_directories(step490_test PRIVATE src)
target_link_libraries(step490_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step491_test tests/step491_test.cpp)
target_include_directories(step491_test PRIVATE src)
target_link_libraries(step491_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step492_test tests/step492_test.cpp)
target_include_directories(step492_test PRIVATE src)
target_link_libraries(step492_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step493_test tests/step493_test.cpp)
target_include_directories(step493_test PRIVATE src)
target_link_libraries(step493_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step494_test tests/step494_test.cpp)
target_include_directories(step494_test PRIVATE src)
target_link_libraries(step494_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step495_test tests/step495_test.cpp)
target_include_directories(step495_test PRIVATE src)
target_link_libraries(step495_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step496_test tests/step496_test.cpp)
target_include_directories(step496_test PRIVATE src)
target_link_libraries(step496_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step497_test tests/step497_test.cpp)
target_include_directories(step497_test PRIVATE src)
target_link_libraries(step497_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step498_test tests/step498_test.cpp)
target_include_directories(step498_test PRIVATE src)
target_link_libraries(step498_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step499_test tests/step499_test.cpp)
target_include_directories(step499_test PRIVATE src)
target_link_libraries(step499_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step500_test tests/step500_test.cpp)
target_include_directories(step500_test PRIVATE src)
target_link_libraries(step500_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step501_test tests/step501_test.cpp)
target_include_directories(step501_test PRIVATE src)
target_link_libraries(step501_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step502_test tests/step502_test.cpp)
target_include_directories(step502_test PRIVATE src)
target_link_libraries(step502_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step503_test tests/step503_test.cpp)
target_include_directories(step503_test PRIVATE src)
target_link_libraries(step503_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step504_test tests/step504_test.cpp)
target_include_directories(step504_test PRIVATE src)
target_link_libraries(step504_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step505_test tests/step505_test.cpp)
target_include_directories(step505_test PRIVATE src)
target_link_libraries(step505_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step506_test tests/step506_test.cpp)
target_include_directories(step506_test PRIVATE src)
target_link_libraries(step506_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step507_test tests/step507_test.cpp)
target_include_directories(step507_test PRIVATE src)
target_link_libraries(step507_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step508_test tests/step508_test.cpp)
target_include_directories(step508_test PRIVATE src)
target_link_libraries(step508_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step509_test tests/step509_test.cpp)
target_include_directories(step509_test PRIVATE src)
target_link_libraries(step509_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step510_test tests/step510_test.cpp)
target_include_directories(step510_test PRIVATE src)
target_link_libraries(step510_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step511_test tests/step511_test.cpp)
target_include_directories(step511_test PRIVATE src)
target_link_libraries(step511_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step512_test tests/step512_test.cpp)
target_include_directories(step512_test PRIVATE src)
target_link_libraries(step512_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step513_test tests/step513_test.cpp)
target_include_directories(step513_test PRIVATE src)
target_link_libraries(step513_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step514_test tests/step514_test.cpp)
target_include_directories(step514_test PRIVATE src)
target_link_libraries(step514_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step515_test tests/step515_test.cpp)
target_include_directories(step515_test PRIVATE src)
target_link_libraries(step515_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step516_test tests/step516_test.cpp)
target_include_directories(step516_test PRIVATE src)
target_link_libraries(step516_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step517_test tests/step517_test.cpp)
target_include_directories(step517_test PRIVATE src)
target_link_libraries(step517_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step518_test tests/step518_test.cpp)
target_include_directories(step518_test PRIVATE src)
target_link_libraries(step518_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step519_test tests/step519_test.cpp)
target_include_directories(step519_test PRIVATE src)
target_link_libraries(step519_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step520_test tests/step520_test.cpp)
target_include_directories(step520_test PRIVATE src)
target_link_libraries(step520_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step521_test tests/step521_test.cpp)
target_include_directories(step521_test PRIVATE src)
target_link_libraries(step521_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step522_test tests/step522_test.cpp)
target_include_directories(step522_test PRIVATE src)
target_link_libraries(step522_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step523_test tests/step523_test.cpp)
target_include_directories(step523_test PRIVATE src)
target_link_libraries(step523_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step524_test tests/step524_test.cpp)
target_include_directories(step524_test PRIVATE src)
target_link_libraries(step524_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step525_test tests/step525_test.cpp)
target_include_directories(step525_test PRIVATE src)
target_link_libraries(step525_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step526_test tests/step526_test.cpp)
target_include_directories(step526_test PRIVATE src)
target_link_libraries(step526_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step527_test tests/step527_test.cpp)
target_include_directories(step527_test PRIVATE src)
target_link_libraries(step527_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step528_test tests/step528_test.cpp)
target_include_directories(step528_test PRIVATE src)
target_link_libraries(step528_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step529_test tests/step529_test.cpp)
target_include_directories(step529_test PRIVATE src)
target_link_libraries(step529_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step530_test tests/step530_test.cpp)
target_include_directories(step530_test PRIVATE src)
target_link_libraries(step530_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step531_test tests/step531_test.cpp)
target_include_directories(step531_test PRIVATE src)
target_link_libraries(step531_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step532_test tests/step532_test.cpp)
target_include_directories(step532_test PRIVATE src)
target_link_libraries(step532_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step533_test tests/step533_test.cpp)
target_include_directories(step533_test PRIVATE src)
target_link_libraries(step533_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step534_test tests/step534_test.cpp)
target_include_directories(step534_test PRIVATE src)
target_link_libraries(step534_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step535_test tests/step535_test.cpp)
target_include_directories(step535_test PRIVATE src)
target_link_libraries(step535_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step536_test tests/step536_test.cpp)
target_include_directories(step536_test PRIVATE src)
target_link_libraries(step536_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step537_test tests/step537_test.cpp)
target_include_directories(step537_test PRIVATE src)
target_link_libraries(step537_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step538_test tests/step538_test.cpp)
target_include_directories(step538_test PRIVATE src)
target_link_libraries(step538_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step539_test tests/step539_test.cpp)
target_include_directories(step539_test PRIVATE src)
target_link_libraries(step539_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step540_test tests/step540_test.cpp)
target_include_directories(step540_test PRIVATE src)
target_link_libraries(step540_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step541_test tests/step541_test.cpp)
target_include_directories(step541_test PRIVATE src)
target_link_libraries(step541_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step542_test tests/step542_test.cpp)
target_include_directories(step542_test PRIVATE src)
target_link_libraries(step542_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step543_test tests/step543_test.cpp)
target_include_directories(step543_test PRIVATE src)
target_link_libraries(step543_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step544_test tests/step544_test.cpp)
target_include_directories(step544_test PRIVATE src)
target_link_libraries(step544_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step545_test tests/step545_test.cpp)
target_include_directories(step545_test PRIVATE src)
target_link_libraries(step545_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step546_test tests/step546_test.cpp)
target_include_directories(step546_test PRIVATE src)
target_link_libraries(step546_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step547_test tests/step547_test.cpp)
target_include_directories(step547_test PRIVATE src)
target_link_libraries(step547_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step548_test tests/step548_test.cpp)
target_include_directories(step548_test PRIVATE src)
target_link_libraries(step548_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step549_test tests/step549_test.cpp)
target_include_directories(step549_test PRIVATE src)
target_link_libraries(step549_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step550_test tests/step550_test.cpp)
target_include_directories(step550_test PRIVATE src)
target_link_libraries(step550_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step551_test tests/step551_test.cpp)
target_include_directories(step551_test PRIVATE src)
target_link_libraries(step551_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step552_test tests/step552_test.cpp)
target_include_directories(step552_test PRIVATE src)
target_link_libraries(step552_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step553_test tests/step553_test.cpp)
target_include_directories(step553_test PRIVATE src)
target_link_libraries(step553_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step554_test tests/step554_test.cpp)
target_include_directories(step554_test PRIVATE src)
target_link_libraries(step554_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step555_test tests/step555_test.cpp)
target_include_directories(step555_test PRIVATE src)
target_link_libraries(step555_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step556_test tests/step556_test.cpp)
target_include_directories(step556_test PRIVATE src)
target_link_libraries(step556_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step557_test tests/step557_test.cpp)
target_include_directories(step557_test PRIVATE src)
target_link_libraries(step557_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step558_test tests/step558_test.cpp)
target_include_directories(step558_test PRIVATE src)
target_link_libraries(step558_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step559_test tests/step559_test.cpp)
target_include_directories(step559_test PRIVATE src)
target_link_libraries(step559_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step560_test tests/step560_test.cpp)
target_include_directories(step560_test PRIVATE src)
target_link_libraries(step560_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step561_test tests/step561_test.cpp)
target_include_directories(step561_test PRIVATE src)
target_link_libraries(step561_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step562_test tests/step562_test.cpp)
target_include_directories(step562_test PRIVATE src)
target_link_libraries(step562_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step563_test tests/step563_test.cpp)
target_include_directories(step563_test PRIVATE src)
target_link_libraries(step563_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step564_test tests/step564_test.cpp)
target_include_directories(step564_test PRIVATE src)
target_link_libraries(step564_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step565_test tests/step565_test.cpp)
target_include_directories(step565_test PRIVATE src)
target_link_libraries(step565_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step566_test tests/step566_test.cpp)
target_include_directories(step566_test PRIVATE src)
target_link_libraries(step566_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step567_test tests/step567_test.cpp)
target_include_directories(step567_test PRIVATE src)
target_link_libraries(step567_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step568_test tests/step568_test.cpp)
target_include_directories(step568_test PRIVATE src)
target_link_libraries(step568_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step569_test tests/step569_test.cpp)
target_include_directories(step569_test PRIVATE src)
target_link_libraries(step569_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step570_test tests/step570_test.cpp)
target_include_directories(step570_test PRIVATE src)
target_link_libraries(step570_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step571_test tests/step571_test.cpp)
target_include_directories(step571_test PRIVATE src)
target_link_libraries(step571_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step572_test tests/step572_test.cpp)
target_include_directories(step572_test PRIVATE src)
target_link_libraries(step572_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step573_test tests/step573_test.cpp)
target_include_directories(step573_test PRIVATE src)
target_link_libraries(step573_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step574_test tests/step574_test.cpp)
target_include_directories(step574_test PRIVATE src)
target_link_libraries(step574_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step575_test tests/step575_test.cpp)
target_include_directories(step575_test PRIVATE src)
target_link_libraries(step575_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step576_test tests/step576_test.cpp)
target_include_directories(step576_test PRIVATE src)
target_link_libraries(step576_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step577_test tests/step577_test.cpp)
target_include_directories(step577_test PRIVATE src)
target_link_libraries(step577_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step578_test tests/step578_test.cpp)
target_include_directories(step578_test PRIVATE src)
target_link_libraries(step578_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step579_test tests/step579_test.cpp)
target_include_directories(step579_test PRIVATE src)
target_link_libraries(step579_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step580_test tests/step580_test.cpp)
target_include_directories(step580_test PRIVATE src)
target_link_libraries(step580_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step581_test tests/step581_test.cpp)
target_include_directories(step581_test PRIVATE src)
target_link_libraries(step581_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step582_test tests/step582_test.cpp)
target_include_directories(step582_test PRIVATE src)
target_link_libraries(step582_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step583_test tests/step583_test.cpp)
target_include_directories(step583_test PRIVATE src)
target_link_libraries(step583_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step584_test tests/step584_test.cpp)
target_include_directories(step584_test PRIVATE src)
target_link_libraries(step584_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step585_test tests/step585_test.cpp)
target_include_directories(step585_test PRIVATE src)
target_link_libraries(step585_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step586_test tests/step586_test.cpp)
target_include_directories(step586_test PRIVATE src)
target_link_libraries(step586_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step587_test tests/step587_test.cpp)
target_include_directories(step587_test PRIVATE src)
target_link_libraries(step587_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step588_test tests/step588_test.cpp)
target_include_directories(step588_test PRIVATE src)
target_link_libraries(step588_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step589_test tests/step589_test.cpp)
target_include_directories(step589_test PRIVATE src)
target_link_libraries(step589_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step590_test tests/step590_test.cpp)
target_include_directories(step590_test PRIVATE src)
target_link_libraries(step590_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step591_test tests/step591_test.cpp)
target_include_directories(step591_test PRIVATE src)
target_link_libraries(step591_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step592_test tests/step592_test.cpp)
target_include_directories(step592_test PRIVATE src)
target_link_libraries(step592_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step593_test tests/step593_test.cpp)
target_include_directories(step593_test PRIVATE src)
target_link_libraries(step593_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step594_test tests/step594_test.cpp)
target_include_directories(step594_test PRIVATE src)
target_link_libraries(step594_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step595_test tests/step595_test.cpp)
target_include_directories(step595_test PRIVATE src)
target_link_libraries(step595_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step596_test tests/step596_test.cpp)
target_include_directories(step596_test PRIVATE src)
target_link_libraries(step596_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step597_test tests/step597_test.cpp)
target_include_directories(step597_test PRIVATE src)
target_link_libraries(step597_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step598_test tests/step598_test.cpp)
target_include_directories(step598_test PRIVATE src)
target_link_libraries(step598_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step599_test tests/step599_test.cpp)
target_include_directories(step599_test PRIVATE src)
target_link_libraries(step599_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step600_test tests/step600_test.cpp)
target_include_directories(step600_test PRIVATE src)
target_link_libraries(step600_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step601_test tests/step601_test.cpp)
target_include_directories(step601_test PRIVATE src)
target_link_libraries(step601_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step602_test tests/step602_test.cpp)
target_include_directories(step602_test PRIVATE src)
target_link_libraries(step602_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step603_test tests/step603_test.cpp)
target_include_directories(step603_test PRIVATE src)
target_link_libraries(step603_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step604_test tests/step604_test.cpp)
target_include_directories(step604_test PRIVATE src)
target_link_libraries(step604_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step605_test tests/step605_test.cpp)
target_include_directories(step605_test PRIVATE src)
target_link_libraries(step605_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step606_test tests/step606_test.cpp)
target_include_directories(step606_test PRIVATE src)
target_link_libraries(step606_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step607_test tests/step607_test.cpp)
target_include_directories(step607_test PRIVATE src)
target_link_libraries(step607_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step608_test tests/step608_test.cpp)
target_include_directories(step608_test PRIVATE src)
target_link_libraries(step608_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step609_test tests/step609_test.cpp)
target_include_directories(step609_test PRIVATE src)
target_link_libraries(step609_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step610_test tests/step610_test.cpp)
target_include_directories(step610_test PRIVATE src)
target_link_libraries(step610_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step611_test tests/step611_test.cpp)
target_include_directories(step611_test PRIVATE src)
target_link_libraries(step611_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step612_test tests/step612_test.cpp)
target_include_directories(step612_test PRIVATE src)
target_link_libraries(step612_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step613_test tests/step613_test.cpp)
target_include_directories(step613_test PRIVATE src)
target_link_libraries(step613_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step614_test tests/step614_test.cpp)
target_include_directories(step614_test PRIVATE src)
target_link_libraries(step614_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step615_test tests/step615_test.cpp)
target_include_directories(step615_test PRIVATE src)
target_link_libraries(step615_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step616_test tests/step616_test.cpp)
target_include_directories(step616_test PRIVATE src)
target_link_libraries(step616_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step617_test tests/step617_test.cpp)
target_include_directories(step617_test PRIVATE src)
target_link_libraries(step617_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step618_test tests/step618_test.cpp)
target_include_directories(step618_test PRIVATE src)
target_link_libraries(step618_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step619_test tests/step619_test.cpp)
target_include_directories(step619_test PRIVATE src)
target_link_libraries(step619_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step620_test tests/step620_test.cpp)
target_include_directories(step620_test PRIVATE src)
target_link_libraries(step620_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step621_test tests/step621_test.cpp)
target_include_directories(step621_test PRIVATE src)
target_link_libraries(step621_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step622_test tests/step622_test.cpp)
target_include_directories(step622_test PRIVATE src)
target_link_libraries(step622_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step623_test tests/step623_test.cpp)
target_include_directories(step623_test PRIVATE src)
target_link_libraries(step623_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step624_test tests/step624_test.cpp)
target_include_directories(step624_test PRIVATE src)
target_link_libraries(step624_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step625_test tests/step625_test.cpp)
target_include_directories(step625_test PRIVATE src)
target_link_libraries(step625_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step626_test tests/step626_test.cpp)
target_include_directories(step626_test PRIVATE src)
target_link_libraries(step626_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step627_test tests/step627_test.cpp)
target_include_directories(step627_test PRIVATE src)
target_link_libraries(step627_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step628_test tests/step628_test.cpp)
target_include_directories(step628_test PRIVATE src)
target_link_libraries(step628_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step629_test tests/step629_test.cpp)
target_include_directories(step629_test PRIVATE src)
target_link_libraries(step629_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step630_test tests/step630_test.cpp)
target_include_directories(step630_test PRIVATE src)
target_link_libraries(step630_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step631_test tests/step631_test.cpp)
target_include_directories(step631_test PRIVATE src)
target_link_libraries(step631_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step632_test tests/step632_test.cpp)
target_include_directories(step632_test PRIVATE src)
target_link_libraries(step632_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step633_test tests/step633_test.cpp)
target_include_directories(step633_test PRIVATE src)
target_link_libraries(step633_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step634_test tests/step634_test.cpp)
target_include_directories(step634_test PRIVATE src)
target_link_libraries(step634_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step635_test tests/step635_test.cpp)
target_include_directories(step635_test PRIVATE src)
target_link_libraries(step635_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step636_test tests/step636_test.cpp)
target_include_directories(step636_test PRIVATE src)
target_link_libraries(step636_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step637_test tests/step637_test.cpp)
target_include_directories(step637_test PRIVATE src)
target_link_libraries(step637_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step638_test tests/step638_test.cpp)
target_include_directories(step638_test PRIVATE src)
target_link_libraries(step638_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step639_test tests/step639_test.cpp)
target_include_directories(step639_test PRIVATE src)
target_link_libraries(step639_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step640_test tests/step640_test.cpp)
target_include_directories(step640_test PRIVATE src)
target_link_libraries(step640_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step641_test tests/step641_test.cpp)
target_include_directories(step641_test PRIVATE src)
target_link_libraries(step641_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step642_test tests/step642_test.cpp)
target_include_directories(step642_test PRIVATE src)
target_link_libraries(step642_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step643_test tests/step643_test.cpp)
target_include_directories(step643_test PRIVATE src)
target_link_libraries(step643_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step644_test tests/step644_test.cpp)
target_include_directories(step644_test PRIVATE src)
target_link_libraries(step644_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step645_test tests/step645_test.cpp)
target_include_directories(step645_test PRIVATE src)
target_link_libraries(step645_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step646_test tests/step646_test.cpp)
target_include_directories(step646_test PRIVATE src)
target_link_libraries(step646_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step647_test tests/step647_test.cpp)
target_include_directories(step647_test PRIVATE src)
target_link_libraries(step647_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step648_test tests/step648_test.cpp)
target_include_directories(step648_test PRIVATE src)
target_link_libraries(step648_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step649_test tests/step649_test.cpp)
target_include_directories(step649_test PRIVATE src)
target_link_libraries(step649_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step650_test tests/step650_test.cpp)
target_include_directories(step650_test PRIVATE src)
target_link_libraries(step650_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step651_test tests/step651_test.cpp)
target_include_directories(step651_test PRIVATE src)
target_link_libraries(step651_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step652_test tests/step652_test.cpp)
target_include_directories(step652_test PRIVATE src)
target_link_libraries(step652_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step653_test tests/step653_test.cpp)
target_include_directories(step653_test PRIVATE src)
target_link_libraries(step653_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step654_test tests/step654_test.cpp)
target_include_directories(step654_test PRIVATE src)
target_link_libraries(step654_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step655_test tests/step655_test.cpp)
target_include_directories(step655_test PRIVATE src)
target_link_libraries(step655_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step656_test tests/step656_test.cpp)
target_include_directories(step656_test PRIVATE src)
target_link_libraries(step656_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step657_test tests/step657_test.cpp)
target_include_directories(step657_test PRIVATE src)
target_link_libraries(step657_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step658_test tests/step658_test.cpp)
target_include_directories(step658_test PRIVATE src)
target_link_libraries(step658_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step659_test tests/step659_test.cpp)
target_include_directories(step659_test PRIVATE src)
target_link_libraries(step659_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step660_test tests/step660_test.cpp)
target_include_directories(step660_test PRIVATE src)
target_link_libraries(step660_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step661_test tests/step661_test.cpp)
target_include_directories(step661_test PRIVATE src)
target_link_libraries(step661_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step662_test tests/step662_test.cpp)
target_include_directories(step662_test PRIVATE src)
target_link_libraries(step662_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step663_test tests/step663_test.cpp)
target_include_directories(step663_test PRIVATE src)
target_link_libraries(step663_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 41: MCP codegen tools wiring (Steps 664-668)
add_executable(step664_test tests/step664_test.cpp)
target_include_directories(step664_test PRIVATE src)
target_link_libraries(step664_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step665_test tests/step665_test.cpp)
target_include_directories(step665_test PRIVATE src)
target_link_libraries(step665_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step666_test tests/step666_test.cpp)
target_include_directories(step666_test PRIVATE src)
target_link_libraries(step666_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step668_test tests/step668_test.cpp)
target_include_directories(step668_test PRIVATE src)
target_link_libraries(step668_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 42: MCP modeling tools wiring (Steps 669-673)
add_executable(step669_test tests/step669_test.cpp)
target_include_directories(step669_test PRIVATE src)
target_link_libraries(step669_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step670_test tests/step670_test.cpp)
target_include_directories(step670_test PRIVATE src)
target_link_libraries(step670_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step671_test tests/step671_test.cpp)
target_include_directories(step671_test PRIVATE src)
target_link_libraries(step671_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step672_test tests/step672_test.cpp)
target_include_directories(step672_test PRIVATE src)
target_link_libraries(step672_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step673_test tests/step673_test.cpp)
target_include_directories(step673_test PRIVATE src)
target_link_libraries(step673_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 43: Cross-file context assembly (Steps 674-678)
add_executable(step674_test tests/step674_test.cpp)
target_include_directories(step674_test PRIVATE src)
target_link_libraries(step674_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step675_test tests/step675_test.cpp)
target_include_directories(step675_test PRIVATE src)
target_link_libraries(step675_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step676_test tests/step676_test.cpp)
target_include_directories(step676_test PRIVATE src)
target_link_libraries(step676_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step677_test tests/step677_test.cpp)
target_include_directories(step677_test PRIVATE src)
target_link_libraries(step677_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step678_test tests/step678_test.cpp)
target_include_directories(step678_test PRIVATE src)
target_link_libraries(step678_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 44: Taskitem self-containment validator (Steps 679-683)
add_executable(step679_test tests/step679_test.cpp)
target_include_directories(step679_test PRIVATE src)
target_link_libraries(step679_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step680_test tests/step680_test.cpp)
target_include_directories(step680_test PRIVATE src)
target_link_libraries(step680_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step681_test tests/step681_test.cpp)
target_include_directories(step681_test PRIVATE src)
target_link_libraries(step681_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step682_test tests/step682_test.cpp)
target_include_directories(step682_test PRIVATE src)
target_link_libraries(step682_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step683_test tests/step683_test.cpp)
target_include_directories(step683_test PRIVATE src)
target_link_libraries(step683_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 45: Agent session metrics + A/B harness (Steps 684-688)
add_executable(step684_test tests/step684_test.cpp)
target_include_directories(step684_test PRIVATE src)
target_link_libraries(step684_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step685_test tests/step685_test.cpp)
target_include_directories(step685_test PRIVATE src)
target_link_libraries(step685_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step686_test tests/step686_test.cpp)
target_include_directories(step686_test PRIVATE src)
target_link_libraries(step686_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step687_test tests/step687_test.cpp)
target_include_directories(step687_test PRIVATE src)
target_link_libraries(step687_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step688_test tests/step688_test.cpp)
target_include_directories(step688_test PRIVATE src)
target_link_libraries(step688_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 46: Semantic core and porting contract (Step 689+)
add_executable(step689_test tests/step689_test.cpp)
target_include_directories(step689_test PRIVATE src)
target_link_libraries(step689_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step690_test tests/step690_test.cpp)
target_include_directories(step690_test PRIVATE src)
target_link_libraries(step690_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step691_test tests/step691_test.cpp)
target_include_directories(step691_test PRIVATE src)
target_link_libraries(step691_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step692_test tests/step692_test.cpp)
target_include_directories(step692_test PRIVATE src)
target_link_libraries(step692_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step693_test tests/step693_test.cpp)
target_include_directories(step693_test PRIVATE src)
target_link_libraries(step693_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step694_test tests/step694_test.cpp)
target_include_directories(step694_test PRIVATE src)
target_link_libraries(step694_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step695_test tests/step695_test.cpp)
target_include_directories(step695_test PRIVATE src)
target_link_libraries(step695_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step696_test tests/step696_test.cpp)
target_include_directories(step696_test PRIVATE src)
target_link_libraries(step696_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step697_test tests/step697_test.cpp)
target_include_directories(step697_test PRIVATE src)
target_link_libraries(step697_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step698_test tests/step698_test.cpp)
target_include_directories(step698_test PRIVATE src)
target_link_libraries(step698_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step699_test tests/step699_test.cpp)
target_include_directories(step699_test PRIVATE src)
target_link_libraries(step699_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step700_test tests/step700_test.cpp)
target_include_directories(step700_test PRIVATE src)
target_link_libraries(step700_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step701_test tests/step701_test.cpp)
target_include_directories(step701_test PRIVATE src)
target_link_libraries(step701_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step702_test tests/step702_test.cpp)
target_include_directories(step702_test PRIVATE src)
target_link_libraries(step702_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step703_test tests/step703_test.cpp)
target_include_directories(step703_test PRIVATE src)
target_link_libraries(step703_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step704_test tests/step704_test.cpp)
target_include_directories(step704_test PRIVATE src)
target_link_libraries(step704_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step705_test tests/step705_test.cpp)
target_include_directories(step705_test PRIVATE src)
target_link_libraries(step705_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step706_test tests/step706_test.cpp)
target_include_directories(step706_test PRIVATE src)
target_link_libraries(step706_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step707_test tests/step707_test.cpp)
target_include_directories(step707_test PRIVATE src)
target_link_libraries(step707_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step708_test tests/step708_test.cpp)
target_include_directories(step708_test PRIVATE src)
target_link_libraries(step708_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step719_test tests/step719_test.cpp)
target_include_directories(step719_test PRIVATE src)
target_link_libraries(step719_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step720_test tests/step720_test.cpp)
target_include_directories(step720_test PRIVATE src)
target_link_libraries(step720_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step721_test tests/step721_test.cpp)
target_include_directories(step721_test PRIVATE src)
target_link_libraries(step721_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step722_test tests/step722_test.cpp)
target_include_directories(step722_test PRIVATE src)
target_link_libraries(step722_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step723_test tests/step723_test.cpp)
target_include_directories(step723_test PRIVATE src)
target_link_libraries(step723_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step724_test tests/step724_test.cpp)
target_include_directories(step724_test PRIVATE src)
target_link_libraries(step724_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step725_test tests/step725_test.cpp)
target_include_directories(step725_test PRIVATE src)
target_link_libraries(step725_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step726_test tests/step726_test.cpp)
target_include_directories(step726_test PRIVATE src)
target_link_libraries(step726_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step727_test tests/step727_test.cpp)
target_include_directories(step727_test PRIVATE src)
target_link_libraries(step727_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step728_test tests/step728_test.cpp)
target_include_directories(step728_test PRIVATE src)
target_link_libraries(step728_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step729_test tests/step729_test.cpp)
target_include_directories(step729_test PRIVATE src)
target_link_libraries(step729_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step730_test tests/step730_test.cpp)
target_include_directories(step730_test PRIVATE src)
target_link_libraries(step730_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step731_test tests/step731_test.cpp)
target_include_directories(step731_test PRIVATE src)
target_link_libraries(step731_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step732_test tests/step732_test.cpp)
target_include_directories(step732_test PRIVATE src)
target_link_libraries(step732_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step733_test tests/step733_test.cpp)
target_include_directories(step733_test PRIVATE src)
target_link_libraries(step733_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step734_test tests/step734_test.cpp)
target_include_directories(step734_test PRIVATE src)
target_link_libraries(step734_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step735_test tests/step735_test.cpp)
target_include_directories(step735_test PRIVATE src)
target_link_libraries(step735_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step736_test tests/step736_test.cpp)
target_include_directories(step736_test PRIVATE src)
target_link_libraries(step736_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step737_test tests/step737_test.cpp)
target_include_directories(step737_test PRIVATE src)
target_link_libraries(step737_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step738_test tests/step738_test.cpp)
target_include_directories(step738_test PRIVATE src)
target_link_libraries(step738_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step739_test tests/step739_test.cpp)
target_include_directories(step739_test PRIVATE src)
target_link_libraries(step739_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step740_test tests/step740_test.cpp)
target_include_directories(step740_test PRIVATE src)
target_link_libraries(step740_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step741_test tests/step741_test.cpp)
target_include_directories(step741_test PRIVATE src)
target_link_libraries(step741_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step742_test tests/step742_test.cpp)
target_include_directories(step742_test PRIVATE src)
target_link_libraries(step742_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step743_test tests/step743_test.cpp)
target_include_directories(step743_test PRIVATE src)
target_link_libraries(step743_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step744_test tests/step744_test.cpp)
target_include_directories(step744_test PRIVATE src)
target_link_libraries(step744_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step745_test tests/step745_test.cpp)
target_include_directories(step745_test PRIVATE src)
target_link_libraries(step745_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step746_test tests/step746_test.cpp)
target_include_directories(step746_test PRIVATE src)
target_link_libraries(step746_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step747_test tests/step747_test.cpp)
target_include_directories(step747_test PRIVATE src)
target_link_libraries(step747_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step748_test tests/step748_test.cpp)
target_include_directories(step748_test PRIVATE src)
target_link_libraries(step748_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step749_test tests/step749_test.cpp)
target_include_directories(step749_test PRIVATE src)
target_link_libraries(step749_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step750_test tests/step750_test.cpp)
target_include_directories(step750_test PRIVATE src)
target_link_libraries(step750_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step751_test tests/step751_test.cpp)
target_include_directories(step751_test PRIVATE src)
target_link_libraries(step751_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step752_test tests/step752_test.cpp)
target_include_directories(step752_test PRIVATE src)
target_link_libraries(step752_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step753_test tests/step753_test.cpp)
target_include_directories(step753_test PRIVATE src)
target_link_libraries(step753_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step754_test tests/step754_test.cpp)
target_include_directories(step754_test PRIVATE src)
target_link_libraries(step754_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step755_test tests/step755_test.cpp)
target_include_directories(step755_test PRIVATE src)
target_link_libraries(step755_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step756_test tests/step756_test.cpp)
target_include_directories(step756_test PRIVATE src)
target_link_libraries(step756_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step757_test tests/step757_test.cpp)
target_include_directories(step757_test PRIVATE src)
target_link_libraries(step757_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step758_test tests/step758_test.cpp)
target_include_directories(step758_test PRIVATE src)
target_link_libraries(step758_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step759_test tests/step759_test.cpp)
target_include_directories(step759_test PRIVATE src)
target_link_libraries(step759_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step760_test tests/step760_test.cpp)
target_include_directories(step760_test PRIVATE src)
target_link_libraries(step760_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step761_test tests/step761_test.cpp)
target_include_directories(step761_test PRIVATE src)
target_link_libraries(step761_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step762_test tests/step762_test.cpp)
target_include_directories(step762_test PRIVATE src)
target_link_libraries(step762_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step763_test tests/step763_test.cpp)
target_include_directories(step763_test PRIVATE src)
target_link_libraries(step763_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step764_test tests/step764_test.cpp)
target_include_directories(step764_test PRIVATE src)
target_link_libraries(step764_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step765_test tests/step765_test.cpp)
target_include_directories(step765_test PRIVATE src)
target_link_libraries(step765_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step766_test tests/step766_test.cpp)
target_include_directories(step766_test PRIVATE src)
target_link_libraries(step766_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step767_test tests/step767_test.cpp)
target_include_directories(step767_test PRIVATE src)
target_link_libraries(step767_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step768_test tests/step768_test.cpp)
target_include_directories(step768_test PRIVATE src)
target_link_libraries(step768_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step769_test tests/step769_test.cpp)
target_include_directories(step769_test PRIVATE src)
target_link_libraries(step769_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step770_test tests/step770_test.cpp)
target_include_directories(step770_test PRIVATE src)
target_link_libraries(step770_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step771_test tests/step771_test.cpp)
target_include_directories(step771_test PRIVATE src)
target_link_libraries(step771_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step772_test tests/step772_test.cpp)
target_include_directories(step772_test PRIVATE src)
target_link_libraries(step772_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step773_test tests/step773_test.cpp)
target_include_directories(step773_test PRIVATE src)
target_link_libraries(step773_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step774_test tests/step774_test.cpp)
target_include_directories(step774_test PRIVATE src)
target_link_libraries(step774_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step775_test tests/step775_test.cpp)
target_include_directories(step775_test PRIVATE src)
target_link_libraries(step775_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step776_test tests/step776_test.cpp)
target_include_directories(step776_test PRIVATE src)
target_link_libraries(step776_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step777_test tests/step777_test.cpp)
target_include_directories(step777_test PRIVATE src)
target_link_libraries(step777_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step778_test tests/step778_test.cpp)
target_include_directories(step778_test PRIVATE src)
target_link_libraries(step778_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step779_test tests/step779_test.cpp)
target_include_directories(step779_test PRIVATE src)
target_link_libraries(step779_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step780_test tests/step780_test.cpp)
target_include_directories(step780_test PRIVATE src)
target_link_libraries(step780_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step781_test tests/step781_test.cpp)
target_include_directories(step781_test PRIVATE src)
target_link_libraries(step781_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step782_test tests/step782_test.cpp)
target_include_directories(step782_test PRIVATE src)
target_link_libraries(step782_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step783_test tests/step783_test.cpp)
target_include_directories(step783_test PRIVATE src)
target_link_libraries(step783_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step784_test tests/step784_test.cpp)
target_include_directories(step784_test PRIVATE src)
target_link_libraries(step784_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step785_test tests/step785_test.cpp)
target_include_directories(step785_test PRIVATE src)
target_link_libraries(step785_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step786_test tests/step786_test.cpp)
target_include_directories(step786_test PRIVATE src)
target_link_libraries(step786_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step787_test tests/step787_test.cpp)
target_include_directories(step787_test PRIVATE src)
target_link_libraries(step787_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step788_test tests/step788_test.cpp)
target_include_directories(step788_test PRIVATE src)
target_link_libraries(step788_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step789_test tests/step789_test.cpp)
target_include_directories(step789_test PRIVATE src)
target_link_libraries(step789_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step790_test tests/step790_test.cpp)
target_include_directories(step790_test PRIVATE src)
target_link_libraries(step790_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step791_test tests/step791_test.cpp)
target_include_directories(step791_test PRIVATE src)
target_link_libraries(step791_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step792_test tests/step792_test.cpp)
target_include_directories(step792_test PRIVATE src)
target_link_libraries(step792_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step793_test tests/step793_test.cpp)
target_include_directories(step793_test PRIVATE src)
target_link_libraries(step793_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step794_test tests/step794_test.cpp)
target_include_directories(step794_test PRIVATE src)
target_link_libraries(step794_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step795_test tests/step795_test.cpp)
target_include_directories(step795_test PRIVATE src)
target_link_libraries(step795_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step796_test tests/step796_test.cpp)
target_include_directories(step796_test PRIVATE src)
target_link_libraries(step796_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step797_test tests/step797_test.cpp)
target_include_directories(step797_test PRIVATE src)
target_link_libraries(step797_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step798_test tests/step798_test.cpp)
target_include_directories(step798_test PRIVATE src)
target_link_libraries(step798_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step799_test tests/step799_test.cpp)
target_include_directories(step799_test PRIVATE src)
target_link_libraries(step799_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step800_test tests/step800_test.cpp)
target_include_directories(step800_test PRIVATE src)
target_link_libraries(step800_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step801_test tests/step801_test.cpp)
target_include_directories(step801_test PRIVATE src)
target_link_libraries(step801_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step802_test tests/step802_test.cpp)
target_include_directories(step802_test PRIVATE src)
target_link_libraries(step802_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step803_test tests/step803_test.cpp)
target_include_directories(step803_test PRIVATE src)
target_link_libraries(step803_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step804_test tests/step804_test.cpp)
target_include_directories(step804_test PRIVATE src)
target_link_libraries(step804_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step805_test tests/step805_test.cpp)
target_include_directories(step805_test PRIVATE src)
target_link_libraries(step805_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step806_test tests/step806_test.cpp)
target_include_directories(step806_test PRIVATE src)
target_link_libraries(step806_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step807_test tests/step807_test.cpp)
target_include_directories(step807_test PRIVATE src)
target_link_libraries(step807_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step808_test tests/step808_test.cpp)
target_include_directories(step808_test PRIVATE src)
target_link_libraries(step808_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step809_test tests/step809_test.cpp)
target_include_directories(step809_test PRIVATE src)
target_link_libraries(step809_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step810_test tests/step810_test.cpp)
target_include_directories(step810_test PRIVATE src)
target_link_libraries(step810_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step811_test tests/step811_test.cpp)
target_include_directories(step811_test PRIVATE src)
target_link_libraries(step811_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step812_test tests/step812_test.cpp)
target_include_directories(step812_test PRIVATE src)
target_link_libraries(step812_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step813_test tests/step813_test.cpp)
target_include_directories(step813_test PRIVATE src)
target_link_libraries(step813_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step814_test tests/step814_test.cpp)
target_include_directories(step814_test PRIVATE src)
target_link_libraries(step814_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step815_test tests/step815_test.cpp)
target_include_directories(step815_test PRIVATE src)
target_link_libraries(step815_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step816_test tests/step816_test.cpp)
target_include_directories(step816_test PRIVATE src)
target_link_libraries(step816_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step817_test tests/step817_test.cpp)
target_include_directories(step817_test PRIVATE src)
target_link_libraries(step817_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step818_test tests/step818_test.cpp)
target_include_directories(step818_test PRIVATE src)
target_link_libraries(step818_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step819_test tests/step819_test.cpp)
target_include_directories(step819_test PRIVATE src)
target_link_libraries(step819_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1449_test tests/step1449_test.cpp)
target_include_directories(step1449_test PRIVATE src)
target_link_libraries(step1449_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1450_test tests/step1450_test.cpp)
target_include_directories(step1450_test PRIVATE src)
target_link_libraries(step1450_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1451_test tests/step1451_test.cpp)
target_include_directories(step1451_test PRIVATE src)
target_link_libraries(step1451_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1452_test tests/step1452_test.cpp)
target_include_directories(step1452_test PRIVATE src)
target_link_libraries(step1452_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1453_test tests/step1453_test.cpp)
target_include_directories(step1453_test PRIVATE src)
target_link_libraries(step1453_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1454_test tests/step1454_test.cpp)
target_include_directories(step1454_test PRIVATE src)
target_link_libraries(step1454_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1455_test tests/step1455_test.cpp)
target_include_directories(step1455_test PRIVATE src)
target_link_libraries(step1455_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1456_test tests/step1456_test.cpp)
target_include_directories(step1456_test PRIVATE src)
target_link_libraries(step1456_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1457_test tests/step1457_test.cpp)
target_include_directories(step1457_test PRIVATE src)
target_link_libraries(step1457_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1458_test tests/step1458_test.cpp)
target_include_directories(step1458_test PRIVATE src)
target_link_libraries(step1458_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1459_test tests/step1459_test.cpp)
target_include_directories(step1459_test PRIVATE src)
target_link_libraries(step1459_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1460_test tests/step1460_test.cpp)
target_include_directories(step1460_test PRIVATE src)
target_link_libraries(step1460_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1461_test tests/step1461_test.cpp)
target_include_directories(step1461_test PRIVATE src)
target_link_libraries(step1461_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1462_test tests/step1462_test.cpp)
target_include_directories(step1462_test PRIVATE src)
target_link_libraries(step1462_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1463_test tests/step1463_test.cpp)
target_include_directories(step1463_test PRIVATE src)
target_link_libraries(step1463_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1464_test tests/step1464_test.cpp)
target_include_directories(step1464_test PRIVATE src)
target_link_libraries(step1464_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1465_test tests/step1465_test.cpp)
target_include_directories(step1465_test PRIVATE src)
target_link_libraries(step1465_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1466_test tests/step1466_test.cpp)
target_include_directories(step1466_test PRIVATE src)
target_link_libraries(step1466_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1467_test tests/step1467_test.cpp)
target_include_directories(step1467_test PRIVATE src)
target_link_libraries(step1467_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1468_test tests/step1468_test.cpp)
target_include_directories(step1468_test PRIVATE src)
target_link_libraries(step1468_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1469_test tests/step1469_test.cpp)
target_include_directories(step1469_test PRIVATE src)
target_link_libraries(step1469_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1470_test tests/step1470_test.cpp)
target_include_directories(step1470_test PRIVATE src)
target_link_libraries(step1470_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1471_test tests/step1471_test.cpp)
target_include_directories(step1471_test PRIVATE src)
target_link_libraries(step1471_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1472_test tests/step1472_test.cpp)
target_include_directories(step1472_test PRIVATE src)
target_link_libraries(step1472_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1473_test tests/step1473_test.cpp)
target_include_directories(step1473_test PRIVATE src)
target_link_libraries(step1473_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1474_test tests/step1474_test.cpp)
target_include_directories(step1474_test PRIVATE src)
target_link_libraries(step1474_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1475_test tests/step1475_test.cpp)
target_include_directories(step1475_test PRIVATE src)
target_link_libraries(step1475_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1476_test tests/step1476_test.cpp)
target_include_directories(step1476_test PRIVATE src)
target_link_libraries(step1476_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1477_test tests/step1477_test.cpp)
target_include_directories(step1477_test PRIVATE src)
target_link_libraries(step1477_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1478_test tests/step1478_test.cpp)
target_include_directories(step1478_test PRIVATE src)
target_link_libraries(step1478_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1479_test tests/step1479_test.cpp)
target_include_directories(step1479_test PRIVATE src)
target_link_libraries(step1479_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1480_test tests/step1480_test.cpp)
target_include_directories(step1480_test PRIVATE src)
target_link_libraries(step1480_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1481_test tests/step1481_test.cpp)
target_include_directories(step1481_test PRIVATE src)
target_link_libraries(step1481_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1482_test tests/step1482_test.cpp)
target_include_directories(step1482_test PRIVATE src)
target_link_libraries(step1482_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1483_test tests/step1483_test.cpp)
target_include_directories(step1483_test PRIVATE src)
target_link_libraries(step1483_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1484_test tests/step1484_test.cpp)
target_include_directories(step1484_test PRIVATE src)
target_link_libraries(step1484_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1485_test tests/step1485_test.cpp)
target_include_directories(step1485_test PRIVATE src)
target_link_libraries(step1485_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1486_test tests/step1486_test.cpp)
target_include_directories(step1486_test PRIVATE src)
target_link_libraries(step1486_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1487_test tests/step1487_test.cpp)
target_include_directories(step1487_test PRIVATE src)
target_link_libraries(step1487_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1488_test tests/step1488_test.cpp)
target_include_directories(step1488_test PRIVATE src)
target_link_libraries(step1488_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1489_test tests/step1489_test.cpp)
target_include_directories(step1489_test PRIVATE src)
target_link_libraries(step1489_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1490_test tests/step1490_test.cpp)
target_include_directories(step1490_test PRIVATE src)
target_link_libraries(step1490_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1491_test tests/step1491_test.cpp)
target_include_directories(step1491_test PRIVATE src)
target_link_libraries(step1491_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1492_test tests/step1492_test.cpp)
target_include_directories(step1492_test PRIVATE src)
target_link_libraries(step1492_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1493_test tests/step1493_test.cpp)
target_include_directories(step1493_test PRIVATE src)
target_link_libraries(step1493_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1494_test tests/step1494_test.cpp)
target_include_directories(step1494_test PRIVATE src)
target_link_libraries(step1494_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1495_test tests/step1495_test.cpp)
target_include_directories(step1495_test PRIVATE src)
target_link_libraries(step1495_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1496_test tests/step1496_test.cpp)
target_include_directories(step1496_test PRIVATE src)
target_link_libraries(step1496_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1497_test tests/step1497_test.cpp)
target_include_directories(step1497_test PRIVATE src)
target_link_libraries(step1497_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1498_test tests/step1498_test.cpp)
target_include_directories(step1498_test PRIVATE src)
target_link_libraries(step1498_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1499_test tests/step1499_test.cpp)
target_include_directories(step1499_test PRIVATE src)
target_link_libraries(step1499_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1500_test tests/step1500_test.cpp)
target_include_directories(step1500_test PRIVATE src)
target_link_libraries(step1500_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1501_test tests/step1501_test.cpp)
target_include_directories(step1501_test PRIVATE src)
target_link_libraries(step1501_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1502_test tests/step1502_test.cpp)
target_include_directories(step1502_test PRIVATE src)
target_link_libraries(step1502_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1503_test tests/step1503_test.cpp)
target_include_directories(step1503_test PRIVATE src)
target_link_libraries(step1503_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1504_test tests/step1504_test.cpp)
target_include_directories(step1504_test PRIVATE src)
target_link_libraries(step1504_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1505_test tests/step1505_test.cpp)
target_include_directories(step1505_test PRIVATE src)
target_link_libraries(step1505_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1506_test tests/step1506_test.cpp)
target_include_directories(step1506_test PRIVATE src)
target_link_libraries(step1506_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1507_test tests/step1507_test.cpp)
target_include_directories(step1507_test PRIVATE src)
target_link_libraries(step1507_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1508_test tests/step1508_test.cpp)
target_include_directories(step1508_test PRIVATE src)
target_link_libraries(step1508_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1509_test tests/step1509_test.cpp)
target_include_directories(step1509_test PRIVATE src)
target_link_libraries(step1509_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1510_test tests/step1510_test.cpp)
target_include_directories(step1510_test PRIVATE src)
target_link_libraries(step1510_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1511_test tests/step1511_test.cpp)
target_include_directories(step1511_test PRIVATE src)
target_link_libraries(step1511_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1512_test tests/step1512_test.cpp)
target_include_directories(step1512_test PRIVATE src)
target_link_libraries(step1512_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1513_test tests/step1513_test.cpp)
target_include_directories(step1513_test PRIVATE src)
target_link_libraries(step1513_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1514_test tests/step1514_test.cpp)
target_include_directories(step1514_test PRIVATE src)
target_link_libraries(step1514_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1515_test tests/step1515_test.cpp)
target_include_directories(step1515_test PRIVATE src)
target_link_libraries(step1515_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1516_test tests/step1516_test.cpp)
target_include_directories(step1516_test PRIVATE src)
target_link_libraries(step1516_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1517_test tests/step1517_test.cpp)
target_include_directories(step1517_test PRIVATE src)
target_link_libraries(step1517_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1518_test tests/step1518_test.cpp)
target_include_directories(step1518_test PRIVATE src)
target_link_libraries(step1518_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1519_test tests/step1519_test.cpp)
target_include_directories(step1519_test PRIVATE src)
target_link_libraries(step1519_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1520_test tests/step1520_test.cpp)
target_include_directories(step1520_test PRIVATE src)
target_link_libraries(step1520_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1521_test tests/step1521_test.cpp)
target_include_directories(step1521_test PRIVATE src)
target_link_libraries(step1521_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1522_test tests/step1522_test.cpp)
target_include_directories(step1522_test PRIVATE src)
target_link_libraries(step1522_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1523_test tests/step1523_test.cpp)
target_include_directories(step1523_test PRIVATE src)
target_link_libraries(step1523_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1524_test tests/step1524_test.cpp)
target_include_directories(step1524_test PRIVATE src)
target_link_libraries(step1524_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1525_test tests/step1525_test.cpp)
target_include_directories(step1525_test PRIVATE src)
target_link_libraries(step1525_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1526_test tests/step1526_test.cpp)
target_include_directories(step1526_test PRIVATE src)
target_link_libraries(step1526_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1527_test tests/step1527_test.cpp)
target_include_directories(step1527_test PRIVATE src)
target_link_libraries(step1527_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1528_test tests/step1528_test.cpp)
target_include_directories(step1528_test PRIVATE src)
target_link_libraries(step1528_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1529_test tests/step1529_test.cpp)
target_include_directories(step1529_test PRIVATE src)
target_link_libraries(step1529_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1530_test tests/step1530_test.cpp)
target_include_directories(step1530_test PRIVATE src)
target_link_libraries(step1530_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1531_test tests/step1531_test.cpp)
target_include_directories(step1531_test PRIVATE src)
target_link_libraries(step1531_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1532_test tests/step1532_test.cpp)
target_include_directories(step1532_test PRIVATE src)
target_link_libraries(step1532_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1533_test tests/step1533_test.cpp)
target_include_directories(step1533_test PRIVATE src)
target_link_libraries(step1533_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1534_test tests/step1534_test.cpp)
target_include_directories(step1534_test PRIVATE src)
target_link_libraries(step1534_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1535_test tests/step1535_test.cpp)
target_include_directories(step1535_test PRIVATE src)
target_link_libraries(step1535_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1536_test tests/step1536_test.cpp)
target_include_directories(step1536_test PRIVATE src)
target_link_libraries(step1536_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1537_test tests/step1537_test.cpp)
target_include_directories(step1537_test PRIVATE src)
target_link_libraries(step1537_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1538_test tests/step1538_test.cpp)
target_include_directories(step1538_test PRIVATE src)
target_link_libraries(step1538_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1539_test tests/step1539_test.cpp)
target_include_directories(step1539_test PRIVATE src)
target_link_libraries(step1539_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1540_test tests/step1540_test.cpp)
target_include_directories(step1540_test PRIVATE src)
target_link_libraries(step1540_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1541_test tests/step1541_test.cpp)
target_include_directories(step1541_test PRIVATE src)
target_link_libraries(step1541_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1542_test tests/step1542_test.cpp)
target_include_directories(step1542_test PRIVATE src)
target_link_libraries(step1542_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1543_test tests/step1543_test.cpp)
target_include_directories(step1543_test PRIVATE src)
target_link_libraries(step1543_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1544_test tests/step1544_test.cpp)
target_include_directories(step1544_test PRIVATE src)
target_link_libraries(step1544_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1545_test tests/step1545_test.cpp)
target_include_directories(step1545_test PRIVATE src)
target_link_libraries(step1545_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1546_test tests/step1546_test.cpp)
target_include_directories(step1546_test PRIVATE src)
target_link_libraries(step1546_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1547_test tests/step1547_test.cpp)
target_include_directories(step1547_test PRIVATE src)
target_link_libraries(step1547_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1548_test tests/step1548_test.cpp)
target_include_directories(step1548_test PRIVATE src)
target_link_libraries(step1548_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Step 12: Dear ImGui shell scaffolding created (main.cpp exists but not built due to dependencies)

# Sprint 59 steps 820-828
add_executable(step820_test tests/step820_test.cpp)
target_include_directories(step820_test PRIVATE src)
target_link_libraries(step820_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step821_test tests/step821_test.cpp)
target_include_directories(step821_test PRIVATE src)
target_link_libraries(step821_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step822_test tests/step822_test.cpp)
target_include_directories(step822_test PRIVATE src)
target_link_libraries(step822_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step823_test tests/step823_test.cpp)
target_include_directories(step823_test PRIVATE src)
target_link_libraries(step823_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step824_test tests/step824_test.cpp)
target_include_directories(step824_test PRIVATE src)
target_link_libraries(step824_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step825_test tests/step825_test.cpp)
target_include_directories(step825_test PRIVATE src)
target_link_libraries(step825_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step826_test tests/step826_test.cpp)
target_include_directories(step826_test PRIVATE src)
target_link_libraries(step826_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step827_test tests/step827_test.cpp)
target_include_directories(step827_test PRIVATE src)
target_link_libraries(step827_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step828_test tests/step828_test.cpp)
target_include_directories(step828_test PRIVATE src)
target_link_libraries(step828_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)


# Sprint 60: Language Graduation and Full-Matrix Release (Steps 829-838)
add_executable(step829_test tests/step829_test.cpp)
target_include_directories(step829_test PRIVATE src)
target_link_libraries(step829_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step830_test tests/step830_test.cpp)
target_include_directories(step830_test PRIVATE src)
target_link_libraries(step830_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step831_test tests/step831_test.cpp)
target_include_directories(step831_test PRIVATE src)
target_link_libraries(step831_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step832_test tests/step832_test.cpp)
target_include_directories(step832_test PRIVATE src)
target_link_libraries(step832_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step833_test tests/step833_test.cpp)
target_include_directories(step833_test PRIVATE src)
target_link_libraries(step833_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step834_test tests/step834_test.cpp)
target_include_directories(step834_test PRIVATE src)
target_link_libraries(step834_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step835_test tests/step835_test.cpp)
target_include_directories(step835_test PRIVATE src)
target_link_libraries(step835_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step836_test tests/step836_test.cpp)
target_include_directories(step836_test PRIVATE src)
target_link_libraries(step836_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step837_test tests/step837_test.cpp)
target_include_directories(step837_test PRIVATE src)
target_link_libraries(step837_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step838_test tests/step838_test.cpp)
target_include_directories(step838_test PRIVATE src)
target_link_libraries(step838_test PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    tree_sitter_python tree_sitter_cpp tree_sitter_elisp
    tree_sitter_javascript tree_sitter_typescript
    tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 61: Continuous Matrix Certification Pipeline (Steps 839-848)
add_executable(step839_test tests/step839_test.cpp)
target_include_directories(step839_test PRIVATE src)
target_link_libraries(step839_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step840_test tests/step840_test.cpp)
target_include_directories(step840_test PRIVATE src)
target_link_libraries(step840_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step841_test tests/step841_test.cpp)
target_include_directories(step841_test PRIVATE src)
target_link_libraries(step841_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step842_test tests/step842_test.cpp)
target_include_directories(step842_test PRIVATE src)
target_link_libraries(step842_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step843_test tests/step843_test.cpp)
target_include_directories(step843_test PRIVATE src)
target_link_libraries(step843_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step844_test tests/step844_test.cpp)
target_include_directories(step844_test PRIVATE src)
target_link_libraries(step844_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step845_test tests/step845_test.cpp)
target_include_directories(step845_test PRIVATE src)
target_link_libraries(step845_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step846_test tests/step846_test.cpp)
target_include_directories(step846_test PRIVATE src)
target_link_libraries(step846_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step847_test tests/step847_test.cpp)
target_include_directories(step847_test PRIVATE src)
target_link_libraries(step847_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step848_test tests/step848_test.cpp)
target_include_directories(step848_test PRIVATE src)
target_link_libraries(step848_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 62: Adapter Quality Telemetry and Failure Taxonomy (Steps 849-858)
add_executable(step849_test tests/step849_test.cpp)
target_include_directories(step849_test PRIVATE src)
target_link_libraries(step849_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step850_test tests/step850_test.cpp)
target_include_directories(step850_test PRIVATE src)
target_link_libraries(step850_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step851_test tests/step851_test.cpp)
target_include_directories(step851_test PRIVATE src)
target_link_libraries(step851_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step852_test tests/step852_test.cpp)
target_include_directories(step852_test PRIVATE src)
target_link_libraries(step852_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step853_test tests/step853_test.cpp)
target_include_directories(step853_test PRIVATE src)
target_link_libraries(step853_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step854_test tests/step854_test.cpp)
target_include_directories(step854_test PRIVATE src)
target_link_libraries(step854_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step855_test tests/step855_test.cpp)
target_include_directories(step855_test PRIVATE src)
target_link_libraries(step855_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step856_test tests/step856_test.cpp)
target_include_directories(step856_test PRIVATE src)
target_link_libraries(step856_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step857_test tests/step857_test.cpp)
target_include_directories(step857_test PRIVATE src)
target_link_libraries(step857_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step858_test tests/step858_test.cpp)
target_include_directories(step858_test PRIVATE src)
target_link_libraries(step858_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 63: Learned Adapter Hints (Steps 859-868)
add_executable(step859_test tests/step859_test.cpp)
target_include_directories(step859_test PRIVATE src)
target_link_libraries(step859_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step860_test tests/step860_test.cpp)
target_include_directories(step860_test PRIVATE src)
target_link_libraries(step860_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step861_test tests/step861_test.cpp)
target_include_directories(step861_test PRIVATE src)
target_link_libraries(step861_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step862_test tests/step862_test.cpp)
target_include_directories(step862_test PRIVATE src)
target_link_libraries(step862_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step863_test tests/step863_test.cpp)
target_include_directories(step863_test PRIVATE src)
target_link_libraries(step863_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step864_test tests/step864_test.cpp)
target_include_directories(step864_test PRIVATE src)
target_link_libraries(step864_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step865_test tests/step865_test.cpp)
target_include_directories(step865_test PRIVATE src)
target_link_libraries(step865_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step866_test tests/step866_test.cpp)
target_include_directories(step866_test PRIVATE src)
target_link_libraries(step866_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step867_test tests/step867_test.cpp)
target_include_directories(step867_test PRIVATE src)
target_link_libraries(step867_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step868_test tests/step868_test.cpp)
target_include_directories(step868_test PRIVATE src)
target_link_libraries(step868_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 64: Cost-Aware Transpilation Planning (Steps 869-878)
add_executable(step869_test tests/step869_test.cpp)
target_include_directories(step869_test PRIVATE src)
target_link_libraries(step869_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step870_test tests/step870_test.cpp)
target_include_directories(step870_test PRIVATE src)
target_link_libraries(step870_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step871_test tests/step871_test.cpp)
target_include_directories(step871_test PRIVATE src)
target_link_libraries(step871_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step872_test tests/step872_test.cpp)
target_include_directories(step872_test PRIVATE src)
target_link_libraries(step872_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step873_test tests/step873_test.cpp)
target_include_directories(step873_test PRIVATE src)
target_link_libraries(step873_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step874_test tests/step874_test.cpp)
target_include_directories(step874_test PRIVATE src)
target_link_libraries(step874_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step875_test tests/step875_test.cpp)
target_include_directories(step875_test PRIVATE src)
target_link_libraries(step875_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step876_test tests/step876_test.cpp)
target_include_directories(step876_test PRIVATE src)
target_link_libraries(step876_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step877_test tests/step877_test.cpp)
target_include_directories(step877_test PRIVATE src)
target_link_libraries(step877_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step878_test tests/step878_test.cpp)
target_include_directories(step878_test PRIVATE src)
target_link_libraries(step878_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

# Sprint 65: Parallel Pair Upgrades and Queue Optimization (Steps 879-888)
add_executable(step879_test tests/step879_test.cpp)
target_include_directories(step879_test PRIVATE src)
target_link_libraries(step879_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step880_test tests/step880_test.cpp)
target_include_directories(step880_test PRIVATE src)
target_link_libraries(step880_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step881_test tests/step881_test.cpp)
target_include_directories(step881_test PRIVATE src)
target_link_libraries(step881_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step882_test tests/step882_test.cpp)
target_include_directories(step882_test PRIVATE src)
target_link_libraries(step882_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step883_test tests/step883_test.cpp)
target_include_directories(step883_test PRIVATE src)
target_link_libraries(step883_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step884_test tests/step884_test.cpp)
target_include_directories(step884_test PRIVATE src)
target_link_libraries(step884_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step885_test tests/step885_test.cpp)
target_include_directories(step885_test PRIVATE src)
target_link_libraries(step885_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step886_test tests/step886_test.cpp)
target_include_directories(step886_test PRIVATE src)
target_link_libraries(step886_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step887_test tests/step887_test.cpp)
target_include_directories(step887_test PRIVATE src)
target_link_libraries(step887_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step888_test tests/step888_test.cpp)
target_include_directories(step888_test PRIVATE src)
target_link_libraries(step888_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step889_test tests/step889_test.cpp)
target_include_directories(step889_test PRIVATE src)
target_link_libraries(step889_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step890_test tests/step890_test.cpp)
target_include_directories(step890_test PRIVATE src)
target_link_libraries(step890_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step891_test tests/step891_test.cpp)
target_include_directories(step891_test PRIVATE src)
target_link_libraries(step891_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step892_test tests/step892_test.cpp)
target_include_directories(step892_test PRIVATE src)
target_link_libraries(step892_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step893_test tests/step893_test.cpp)
target_include_directories(step893_test PRIVATE src)
target_link_libraries(step893_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step894_test tests/step894_test.cpp)
target_include_directories(step894_test PRIVATE src)
target_link_libraries(step894_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step895_test tests/step895_test.cpp)
target_include_directories(step895_test PRIVATE src)
target_link_libraries(step895_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step896_test tests/step896_test.cpp)
target_include_directories(step896_test PRIVATE src)
target_link_libraries(step896_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step897_test tests/step897_test.cpp)
target_include_directories(step897_test PRIVATE src)
target_link_libraries(step897_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step898_test tests/step898_test.cpp)
target_include_directories(step898_test PRIVATE src)
target_link_libraries(step898_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step899_test tests/step899_test.cpp)
target_include_directories(step899_test PRIVATE src)
target_link_libraries(step899_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step900_test tests/step900_test.cpp)
target_include_directories(step900_test PRIVATE src)
target_link_libraries(step900_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step901_test tests/step901_test.cpp)
target_include_directories(step901_test PRIVATE src)
target_link_libraries(step901_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step902_test tests/step902_test.cpp)
target_include_directories(step902_test PRIVATE src)
target_link_libraries(step902_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step903_test tests/step903_test.cpp)
target_include_directories(step903_test PRIVATE src)
target_link_libraries(step903_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step904_test tests/step904_test.cpp)
target_include_directories(step904_test PRIVATE src)
target_link_libraries(step904_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step905_test tests/step905_test.cpp)
target_include_directories(step905_test PRIVATE src)
target_link_libraries(step905_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step906_test tests/step906_test.cpp)
target_include_directories(step906_test PRIVATE src)
target_link_libraries(step906_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step907_test tests/step907_test.cpp)
target_include_directories(step907_test PRIVATE src)
target_link_libraries(step907_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step908_test tests/step908_test.cpp)
target_include_directories(step908_test PRIVATE src)
target_link_libraries(step908_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step909_test tests/step909_test.cpp)
target_include_directories(step909_test PRIVATE src)
target_link_libraries(step909_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step910_test tests/step910_test.cpp)
target_include_directories(step910_test PRIVATE src)
target_link_libraries(step910_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step911_test tests/step911_test.cpp)
target_include_directories(step911_test PRIVATE src)
target_link_libraries(step911_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step912_test tests/step912_test.cpp)
target_include_directories(step912_test PRIVATE src)
target_link_libraries(step912_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step913_test tests/step913_test.cpp)
target_include_directories(step913_test PRIVATE src)
target_link_libraries(step913_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step914_test tests/step914_test.cpp)
target_include_directories(step914_test PRIVATE src)
target_link_libraries(step914_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step915_test tests/step915_test.cpp)
target_include_directories(step915_test PRIVATE src)
target_link_libraries(step915_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step916_test tests/step916_test.cpp)
target_include_directories(step916_test PRIVATE src)
target_link_libraries(step916_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step917_test tests/step917_test.cpp)
target_include_directories(step917_test PRIVATE src)
target_link_libraries(step917_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step918_test tests/step918_test.cpp)
target_include_directories(step918_test PRIVATE src)
target_link_libraries(step918_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step919_test tests/step919_test.cpp)
target_include_directories(step919_test PRIVATE src)
target_link_libraries(step919_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step920_test tests/step920_test.cpp)
target_include_directories(step920_test PRIVATE src)
target_link_libraries(step920_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step921_test tests/step921_test.cpp)
target_include_directories(step921_test PRIVATE src)
target_link_libraries(step921_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step922_test tests/step922_test.cpp)
target_include_directories(step922_test PRIVATE src)
target_link_libraries(step922_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step923_test tests/step923_test.cpp)
target_include_directories(step923_test PRIVATE src)
target_link_libraries(step923_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step924_test tests/step924_test.cpp)
target_include_directories(step924_test PRIVATE src)
target_link_libraries(step924_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step925_test tests/step925_test.cpp)
target_include_directories(step925_test PRIVATE src)
target_link_libraries(step925_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step926_test tests/step926_test.cpp)
target_include_directories(step926_test PRIVATE src)
target_link_libraries(step926_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step927_test tests/step927_test.cpp)
target_include_directories(step927_test PRIVATE src)
target_link_libraries(step927_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step928_test tests/step928_test.cpp)
target_include_directories(step928_test PRIVATE src)
target_link_libraries(step928_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step929_test tests/step929_test.cpp)
target_include_directories(step929_test PRIVATE src)
target_link_libraries(step929_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step930_test tests/step930_test.cpp)
target_include_directories(step930_test PRIVATE src)
target_link_libraries(step930_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step931_test tests/step931_test.cpp)
target_include_directories(step931_test PRIVATE src)
target_link_libraries(step931_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step932_test tests/step932_test.cpp)
target_include_directories(step932_test PRIVATE src)
target_link_libraries(step932_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step933_test tests/step933_test.cpp)
target_include_directories(step933_test PRIVATE src)
target_link_libraries(step933_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step934_test tests/step934_test.cpp)
target_include_directories(step934_test PRIVATE src)
target_link_libraries(step934_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step935_test tests/step935_test.cpp)
target_include_directories(step935_test PRIVATE src)
target_link_libraries(step935_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step936_test tests/step936_test.cpp)
target_include_directories(step936_test PRIVATE src)
target_link_libraries(step936_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step937_test tests/step937_test.cpp)
target_include_directories(step937_test PRIVATE src)
target_link_libraries(step937_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step938_test tests/step938_test.cpp)
target_include_directories(step938_test PRIVATE src)
target_link_libraries(step938_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step939_test tests/step939_test.cpp)
target_include_directories(step939_test PRIVATE src)
target_link_libraries(step939_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step940_test tests/step940_test.cpp)
target_include_directories(step940_test PRIVATE src)
target_link_libraries(step940_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step941_test tests/step941_test.cpp)
target_include_directories(step941_test PRIVATE src)
target_link_libraries(step941_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step942_test tests/step942_test.cpp)
target_include_directories(step942_test PRIVATE src)
target_link_libraries(step942_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step943_test tests/step943_test.cpp)
target_include_directories(step943_test PRIVATE src)
target_link_libraries(step943_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step944_test tests/step944_test.cpp)
target_include_directories(step944_test PRIVATE src)
target_link_libraries(step944_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step945_test tests/step945_test.cpp)
target_include_directories(step945_test PRIVATE src)
target_link_libraries(step945_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step946_test tests/step946_test.cpp)
target_include_directories(step946_test PRIVATE src)
target_link_libraries(step946_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step947_test tests/step947_test.cpp)
target_include_directories(step947_test PRIVATE src)
target_link_libraries(step947_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step948_test tests/step948_test.cpp)
target_include_directories(step948_test PRIVATE src)
target_link_libraries(step948_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step949_test tests/step949_test.cpp)
target_include_directories(step949_test PRIVATE src)
target_link_libraries(step949_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step950_test tests/step950_test.cpp)
target_include_directories(step950_test PRIVATE src)
target_link_libraries(step950_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step951_test tests/step951_test.cpp)
target_include_directories(step951_test PRIVATE src)
target_link_libraries(step951_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step952_test tests/step952_test.cpp)
target_include_directories(step952_test PRIVATE src)
target_link_libraries(step952_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step953_test tests/step953_test.cpp)
target_include_directories(step953_test PRIVATE src)
target_link_libraries(step953_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step954_test tests/step954_test.cpp)
target_include_directories(step954_test PRIVATE src)
target_link_libraries(step954_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step955_test tests/step955_test.cpp)
target_include_directories(step955_test PRIVATE src)
target_link_libraries(step955_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step956_test tests/step956_test.cpp)
target_include_directories(step956_test PRIVATE src)
target_link_libraries(step956_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step957_test tests/step957_test.cpp)
target_include_directories(step957_test PRIVATE src)
target_link_libraries(step957_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step958_test tests/step958_test.cpp)
target_include_directories(step958_test PRIVATE src)
target_link_libraries(step958_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step959_test tests/step959_test.cpp)
target_include_directories(step959_test PRIVATE src)
target_link_libraries(step959_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step960_test tests/step960_test.cpp)
target_include_directories(step960_test PRIVATE src)
target_link_libraries(step960_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step961_test tests/step961_test.cpp)
target_include_directories(step961_test PRIVATE src)
target_link_libraries(step961_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step962_test tests/step962_test.cpp)
target_include_directories(step962_test PRIVATE src)
target_link_libraries(step962_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step963_test tests/step963_test.cpp)
target_include_directories(step963_test PRIVATE src)
target_link_libraries(step963_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step964_test tests/step964_test.cpp)
target_include_directories(step964_test PRIVATE src)
target_link_libraries(step964_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step965_test tests/step965_test.cpp)
target_include_directories(step965_test PRIVATE src)
target_link_libraries(step965_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step966_test tests/step966_test.cpp)
target_include_directories(step966_test PRIVATE src)
target_link_libraries(step966_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step967_test tests/step967_test.cpp)
target_include_directories(step967_test PRIVATE src)
target_link_libraries(step967_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step968_test tests/step968_test.cpp)
target_include_directories(step968_test PRIVATE src)
target_link_libraries(step968_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step969_test tests/step969_test.cpp)
target_include_directories(step969_test PRIVATE src)
target_link_libraries(step969_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step970_test tests/step970_test.cpp)
target_include_directories(step970_test PRIVATE src)
target_link_libraries(step970_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step971_test tests/step971_test.cpp)
target_include_directories(step971_test PRIVATE src)
target_link_libraries(step971_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step972_test tests/step972_test.cpp)
target_include_directories(step972_test PRIVATE src)
target_link_libraries(step972_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step973_test tests/step973_test.cpp)
target_include_directories(step973_test PRIVATE src)
target_link_libraries(step973_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step974_test tests/step974_test.cpp)
target_include_directories(step974_test PRIVATE src)
target_link_libraries(step974_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step975_test tests/step975_test.cpp)
target_include_directories(step975_test PRIVATE src)
target_link_libraries(step975_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step976_test tests/step976_test.cpp)
target_include_directories(step976_test PRIVATE src)
target_link_libraries(step976_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step977_test tests/step977_test.cpp)
target_include_directories(step977_test PRIVATE src)
target_link_libraries(step977_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step978_test tests/step978_test.cpp)
target_include_directories(step978_test PRIVATE src)
target_link_libraries(step978_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step979_test tests/step979_test.cpp)
target_include_directories(step979_test PRIVATE src)
target_link_libraries(step979_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step980_test tests/step980_test.cpp)
target_include_directories(step980_test PRIVATE src)
target_link_libraries(step980_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step981_test tests/step981_test.cpp)
target_include_directories(step981_test PRIVATE src)
target_link_libraries(step981_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step982_test tests/step982_test.cpp)
target_include_directories(step982_test PRIVATE src)
target_link_libraries(step982_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step983_test tests/step983_test.cpp)
target_include_directories(step983_test PRIVATE src)
target_link_libraries(step983_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step984_test tests/step984_test.cpp)
target_include_directories(step984_test PRIVATE src)
target_link_libraries(step984_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step985_test tests/step985_test.cpp)
target_include_directories(step985_test PRIVATE src)
target_link_libraries(step985_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step986_test tests/step986_test.cpp)
target_include_directories(step986_test PRIVATE src)
target_link_libraries(step986_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step987_test tests/step987_test.cpp)
target_include_directories(step987_test PRIVATE src)
target_link_libraries(step987_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step988_test tests/step988_test.cpp)
target_include_directories(step988_test PRIVATE src)
target_link_libraries(step988_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step989_test tests/step989_test.cpp)
target_include_directories(step989_test PRIVATE src)
target_link_libraries(step989_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step990_test tests/step990_test.cpp)
target_include_directories(step990_test PRIVATE src)
target_link_libraries(step990_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step991_test tests/step991_test.cpp)
target_include_directories(step991_test PRIVATE src)
target_link_libraries(step991_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step992_test tests/step992_test.cpp)
target_include_directories(step992_test PRIVATE src)
target_link_libraries(step992_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step993_test tests/step993_test.cpp)
target_include_directories(step993_test PRIVATE src)
target_link_libraries(step993_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step994_test tests/step994_test.cpp)
target_include_directories(step994_test PRIVATE src)
target_link_libraries(step994_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step995_test tests/step995_test.cpp)
target_include_directories(step995_test PRIVATE src)
target_link_libraries(step995_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step996_test tests/step996_test.cpp)
target_include_directories(step996_test PRIVATE src)
target_link_libraries(step996_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step997_test tests/step997_test.cpp)
target_include_directories(step997_test PRIVATE src)
target_link_libraries(step997_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step998_test tests/step998_test.cpp)
target_include_directories(step998_test PRIVATE src)
target_link_libraries(step998_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step999_test tests/step999_test.cpp)
target_include_directories(step999_test PRIVATE src)
target_link_libraries(step999_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1000_test tests/step1000_test.cpp)
target_include_directories(step1000_test PRIVATE src)
target_link_libraries(step1000_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1001_test tests/step1001_test.cpp)
target_include_directories(step1001_test PRIVATE src)
target_link_libraries(step1001_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1002_test tests/step1002_test.cpp)
target_include_directories(step1002_test PRIVATE src)
target_link_libraries(step1002_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1003_test tests/step1003_test.cpp)
target_include_directories(step1003_test PRIVATE src)
target_link_libraries(step1003_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1004_test tests/step1004_test.cpp)
target_include_directories(step1004_test PRIVATE src)
target_link_libraries(step1004_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1005_test tests/step1005_test.cpp)
target_include_directories(step1005_test PRIVATE src)
target_link_libraries(step1005_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1006_test tests/step1006_test.cpp)
target_include_directories(step1006_test PRIVATE src)
target_link_libraries(step1006_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1007_test tests/step1007_test.cpp)
target_include_directories(step1007_test PRIVATE src)
target_link_libraries(step1007_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1008_test tests/step1008_test.cpp)
target_include_directories(step1008_test PRIVATE src)
target_link_libraries(step1008_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1009_test tests/step1009_test.cpp)
target_include_directories(step1009_test PRIVATE src)
target_link_libraries(step1009_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1010_test tests/step1010_test.cpp)
target_include_directories(step1010_test PRIVATE src)
target_link_libraries(step1010_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1011_test tests/step1011_test.cpp)
target_include_directories(step1011_test PRIVATE src)
target_link_libraries(step1011_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1012_test tests/step1012_test.cpp)
target_include_directories(step1012_test PRIVATE src)
target_link_libraries(step1012_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1013_test tests/step1013_test.cpp)
target_include_directories(step1013_test PRIVATE src)
target_link_libraries(step1013_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1014_test tests/step1014_test.cpp)
target_include_directories(step1014_test PRIVATE src)
target_link_libraries(step1014_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1015_test tests/step1015_test.cpp)
target_include_directories(step1015_test PRIVATE src)
target_link_libraries(step1015_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1016_test tests/step1016_test.cpp)
target_include_directories(step1016_test PRIVATE src)
target_link_libraries(step1016_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1017_test tests/step1017_test.cpp)
target_include_directories(step1017_test PRIVATE src)
target_link_libraries(step1017_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1018_test tests/step1018_test.cpp)
target_include_directories(step1018_test PRIVATE src)
target_link_libraries(step1018_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1019_test tests/step1019_test.cpp)
target_include_directories(step1019_test PRIVATE src)
target_link_libraries(step1019_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1020_test tests/step1020_test.cpp)
target_include_directories(step1020_test PRIVATE src)
target_link_libraries(step1020_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1021_test tests/step1021_test.cpp)
target_include_directories(step1021_test PRIVATE src)
target_link_libraries(step1021_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1022_test tests/step1022_test.cpp)
target_include_directories(step1022_test PRIVATE src)
target_link_libraries(step1022_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1023_test tests/step1023_test.cpp)
target_include_directories(step1023_test PRIVATE src)
target_link_libraries(step1023_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1024_test tests/step1024_test.cpp)
target_include_directories(step1024_test PRIVATE src)
target_link_libraries(step1024_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1025_test tests/step1025_test.cpp)
target_include_directories(step1025_test PRIVATE src)
target_link_libraries(step1025_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1026_test tests/step1026_test.cpp)
target_include_directories(step1026_test PRIVATE src)
target_link_libraries(step1026_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1027_test tests/step1027_test.cpp)
target_include_directories(step1027_test PRIVATE src)
target_link_libraries(step1027_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1028_test tests/step1028_test.cpp)
target_include_directories(step1028_test PRIVATE src)
target_link_libraries(step1028_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1029_test tests/step1029_test.cpp)
target_include_directories(step1029_test PRIVATE src)
target_link_libraries(step1029_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1030_test tests/step1030_test.cpp)
target_include_directories(step1030_test PRIVATE src)
target_link_libraries(step1030_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1031_test tests/step1031_test.cpp)
target_include_directories(step1031_test PRIVATE src)
target_link_libraries(step1031_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1032_test tests/step1032_test.cpp)
target_include_directories(step1032_test PRIVATE src)
target_link_libraries(step1032_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1033_test tests/step1033_test.cpp)
target_include_directories(step1033_test PRIVATE src)
target_link_libraries(step1033_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1034_test tests/step1034_test.cpp)
target_include_directories(step1034_test PRIVATE src)
target_link_libraries(step1034_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1035_test tests/step1035_test.cpp)
target_include_directories(step1035_test PRIVATE src)
target_link_libraries(step1035_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1036_test tests/step1036_test.cpp)
target_include_directories(step1036_test PRIVATE src)
target_link_libraries(step1036_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1037_test tests/step1037_test.cpp)
target_include_directories(step1037_test PRIVATE src)
target_link_libraries(step1037_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1038_test tests/step1038_test.cpp)
target_include_directories(step1038_test PRIVATE src)
target_link_libraries(step1038_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1039_test tests/step1039_test.cpp)
target_include_directories(step1039_test PRIVATE src)
target_link_libraries(step1039_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1040_test tests/step1040_test.cpp)
target_include_directories(step1040_test PRIVATE src)
target_link_libraries(step1040_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1041_test tests/step1041_test.cpp)
target_include_directories(step1041_test PRIVATE src)
target_link_libraries(step1041_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1042_test tests/step1042_test.cpp)
target_include_directories(step1042_test PRIVATE src)
target_link_libraries(step1042_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1043_test tests/step1043_test.cpp)
target_include_directories(step1043_test PRIVATE src)
target_link_libraries(step1043_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1044_test tests/step1044_test.cpp)
target_include_directories(step1044_test PRIVATE src)
target_link_libraries(step1044_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1045_test tests/step1045_test.cpp)
target_include_directories(step1045_test PRIVATE src)
target_link_libraries(step1045_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1046_test tests/step1046_test.cpp)
target_include_directories(step1046_test PRIVATE src)
target_link_libraries(step1046_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1047_test tests/step1047_test.cpp)
target_include_directories(step1047_test PRIVATE src)
target_link_libraries(step1047_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1048_test tests/step1048_test.cpp)
target_include_directories(step1048_test PRIVATE src)
target_link_libraries(step1048_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1049_test tests/step1049_test.cpp)
target_include_directories(step1049_test PRIVATE src)
target_link_libraries(step1049_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1050_test tests/step1050_test.cpp)
target_include_directories(step1050_test PRIVATE src)
target_link_libraries(step1050_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1051_test tests/step1051_test.cpp)
target_include_directories(step1051_test PRIVATE src)
target_link_libraries(step1051_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1052_test tests/step1052_test.cpp)
target_include_directories(step1052_test PRIVATE src)
target_link_libraries(step1052_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1053_test tests/step1053_test.cpp)
target_include_directories(step1053_test PRIVATE src)
target_link_libraries(step1053_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1054_test tests/step1054_test.cpp)
target_include_directories(step1054_test PRIVATE src)
target_link_libraries(step1054_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1055_test tests/step1055_test.cpp)
target_include_directories(step1055_test PRIVATE src)
target_link_libraries(step1055_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1056_test tests/step1056_test.cpp)
target_include_directories(step1056_test PRIVATE src)
target_link_libraries(step1056_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1057_test tests/step1057_test.cpp)
target_include_directories(step1057_test PRIVATE src)
target_link_libraries(step1057_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1058_test tests/step1058_test.cpp)
target_include_directories(step1058_test PRIVATE src)
target_link_libraries(step1058_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1059_test tests/step1059_test.cpp)
target_include_directories(step1059_test PRIVATE src)
target_link_libraries(step1059_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1060_test tests/step1060_test.cpp)
target_include_directories(step1060_test PRIVATE src)
target_link_libraries(step1060_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1061_test tests/step1061_test.cpp)
target_include_directories(step1061_test PRIVATE src)
target_link_libraries(step1061_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1062_test tests/step1062_test.cpp)
target_include_directories(step1062_test PRIVATE src)
target_link_libraries(step1062_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1063_test tests/step1063_test.cpp)
target_include_directories(step1063_test PRIVATE src)
target_link_libraries(step1063_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1064_test tests/step1064_test.cpp)
target_include_directories(step1064_test PRIVATE src)
target_link_libraries(step1064_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1065_test tests/step1065_test.cpp)
target_include_directories(step1065_test PRIVATE src)
target_link_libraries(step1065_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1066_test tests/step1066_test.cpp)
target_include_directories(step1066_test PRIVATE src)
target_link_libraries(step1066_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1067_test tests/step1067_test.cpp)
target_include_directories(step1067_test PRIVATE src)
target_link_libraries(step1067_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1068_test tests/step1068_test.cpp)
target_include_directories(step1068_test PRIVATE src)
target_link_libraries(step1068_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1069_test tests/step1069_test.cpp)
target_include_directories(step1069_test PRIVATE src)
target_link_libraries(step1069_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1070_test tests/step1070_test.cpp)
target_include_directories(step1070_test PRIVATE src)
target_link_libraries(step1070_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1071_test tests/step1071_test.cpp)
target_include_directories(step1071_test PRIVATE src)
target_link_libraries(step1071_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1072_test tests/step1072_test.cpp)
target_include_directories(step1072_test PRIVATE src)
target_link_libraries(step1072_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1073_test tests/step1073_test.cpp)
target_include_directories(step1073_test PRIVATE src)
target_link_libraries(step1073_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1074_test tests/step1074_test.cpp)
target_include_directories(step1074_test PRIVATE src)
target_link_libraries(step1074_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1075_test tests/step1075_test.cpp)
target_include_directories(step1075_test PRIVATE src)
target_link_libraries(step1075_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1076_test tests/step1076_test.cpp)
target_include_directories(step1076_test PRIVATE src)
target_link_libraries(step1076_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1077_test tests/step1077_test.cpp)
target_include_directories(step1077_test PRIVATE src)
target_link_libraries(step1077_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1078_test tests/step1078_test.cpp)
target_include_directories(step1078_test PRIVATE src)
target_link_libraries(step1078_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1079_test tests/step1079_test.cpp)
target_include_directories(step1079_test PRIVATE src)
target_link_libraries(step1079_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1080_test tests/step1080_test.cpp)
target_include_directories(step1080_test PRIVATE src)
target_link_libraries(step1080_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1081_test tests/step1081_test.cpp)
target_include_directories(step1081_test PRIVATE src)
target_link_libraries(step1081_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1082_test tests/step1082_test.cpp)
target_include_directories(step1082_test PRIVATE src)
target_link_libraries(step1082_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1083_test tests/step1083_test.cpp)
target_include_directories(step1083_test PRIVATE src)
target_link_libraries(step1083_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1084_test tests/step1084_test.cpp)
target_include_directories(step1084_test PRIVATE src)
target_link_libraries(step1084_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1085_test tests/step1085_test.cpp)
target_include_directories(step1085_test PRIVATE src)
target_link_libraries(step1085_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1086_test tests/step1086_test.cpp)
target_include_directories(step1086_test PRIVATE src)
target_link_libraries(step1086_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1087_test tests/step1087_test.cpp)
target_include_directories(step1087_test PRIVATE src)
target_link_libraries(step1087_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1088_test tests/step1088_test.cpp)
target_include_directories(step1088_test PRIVATE src)
target_link_libraries(step1088_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1089_test tests/step1089_test.cpp)
target_include_directories(step1089_test PRIVATE src)
target_link_libraries(step1089_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1090_test tests/step1090_test.cpp)
target_include_directories(step1090_test PRIVATE src)
target_link_libraries(step1090_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1091_test tests/step1091_test.cpp)
target_include_directories(step1091_test PRIVATE src)
target_link_libraries(step1091_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1092_test tests/step1092_test.cpp)
target_include_directories(step1092_test PRIVATE src)
target_link_libraries(step1092_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1093_test tests/step1093_test.cpp)
target_include_directories(step1093_test PRIVATE src)
target_link_libraries(step1093_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1094_test tests/step1094_test.cpp)
target_include_directories(step1094_test PRIVATE src)
target_link_libraries(step1094_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1095_test tests/step1095_test.cpp)
target_include_directories(step1095_test PRIVATE src)
target_link_libraries(step1095_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1096_test tests/step1096_test.cpp)
target_include_directories(step1096_test PRIVATE src)
target_link_libraries(step1096_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1097_test tests/step1097_test.cpp)
target_include_directories(step1097_test PRIVATE src)
target_link_libraries(step1097_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1098_test tests/step1098_test.cpp)
target_include_directories(step1098_test PRIVATE src)
target_link_libraries(step1098_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1099_test tests/step1099_test.cpp)
target_include_directories(step1099_test PRIVATE src)
target_link_libraries(step1099_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1100_test tests/step1100_test.cpp)
target_include_directories(step1100_test PRIVATE src)
target_link_libraries(step1100_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1101_test tests/step1101_test.cpp)
target_include_directories(step1101_test PRIVATE src)
target_link_libraries(step1101_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1102_test tests/step1102_test.cpp)
target_include_directories(step1102_test PRIVATE src)
target_link_libraries(step1102_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1103_test tests/step1103_test.cpp)
target_include_directories(step1103_test PRIVATE src)
target_link_libraries(step1103_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1104_test tests/step1104_test.cpp)
target_include_directories(step1104_test PRIVATE src)
target_link_libraries(step1104_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1105_test tests/step1105_test.cpp)
target_include_directories(step1105_test PRIVATE src)
target_link_libraries(step1105_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1106_test tests/step1106_test.cpp)
target_include_directories(step1106_test PRIVATE src)
target_link_libraries(step1106_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1107_test tests/step1107_test.cpp)
target_include_directories(step1107_test PRIVATE src)
target_link_libraries(step1107_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1108_test tests/step1108_test.cpp)
target_include_directories(step1108_test PRIVATE src)
target_link_libraries(step1108_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1109_test tests/step1109_test.cpp)
target_include_directories(step1109_test PRIVATE src)
target_link_libraries(step1109_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1110_test tests/step1110_test.cpp)
target_include_directories(step1110_test PRIVATE src)
target_link_libraries(step1110_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1111_test tests/step1111_test.cpp)
target_include_directories(step1111_test PRIVATE src)
target_link_libraries(step1111_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1112_test tests/step1112_test.cpp)
target_include_directories(step1112_test PRIVATE src)
target_link_libraries(step1112_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1113_test tests/step1113_test.cpp)
target_include_directories(step1113_test PRIVATE src)
target_link_libraries(step1113_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1114_test tests/step1114_test.cpp)
target_include_directories(step1114_test PRIVATE src)
target_link_libraries(step1114_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1115_test tests/step1115_test.cpp)
target_include_directories(step1115_test PRIVATE src)
target_link_libraries(step1115_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1116_test tests/step1116_test.cpp)
target_include_directories(step1116_test PRIVATE src)
target_link_libraries(step1116_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1117_test tests/step1117_test.cpp)
target_include_directories(step1117_test PRIVATE src)
target_link_libraries(step1117_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1118_test tests/step1118_test.cpp)
target_include_directories(step1118_test PRIVATE src)
target_link_libraries(step1118_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1119_test tests/step1119_test.cpp)
target_include_directories(step1119_test PRIVATE src)
target_link_libraries(step1119_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1120_test tests/step1120_test.cpp)
target_include_directories(step1120_test PRIVATE src)
target_link_libraries(step1120_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1121_test tests/step1121_test.cpp)
target_include_directories(step1121_test PRIVATE src)
target_link_libraries(step1121_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1122_test tests/step1122_test.cpp)
target_include_directories(step1122_test PRIVATE src)
target_link_libraries(step1122_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1123_test tests/step1123_test.cpp)
target_include_directories(step1123_test PRIVATE src)
target_link_libraries(step1123_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1124_test tests/step1124_test.cpp)
target_include_directories(step1124_test PRIVATE src)
target_link_libraries(step1124_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1125_test tests/step1125_test.cpp)
target_include_directories(step1125_test PRIVATE src)
target_link_libraries(step1125_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1126_test tests/step1126_test.cpp)
target_include_directories(step1126_test PRIVATE src)
target_link_libraries(step1126_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1127_test tests/step1127_test.cpp)
target_include_directories(step1127_test PRIVATE src)
target_link_libraries(step1127_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1128_test tests/step1128_test.cpp)
target_include_directories(step1128_test PRIVATE src)
target_link_libraries(step1128_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1129_test tests/step1129_test.cpp)
target_include_directories(step1129_test PRIVATE src)
target_link_libraries(step1129_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1130_test tests/step1130_test.cpp)
target_include_directories(step1130_test PRIVATE src)
target_link_libraries(step1130_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1131_test tests/step1131_test.cpp)
target_include_directories(step1131_test PRIVATE src)
target_link_libraries(step1131_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1132_test tests/step1132_test.cpp)
target_include_directories(step1132_test PRIVATE src)
target_link_libraries(step1132_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1133_test tests/step1133_test.cpp)
target_include_directories(step1133_test PRIVATE src)
target_link_libraries(step1133_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1134_test tests/step1134_test.cpp)
target_include_directories(step1134_test PRIVATE src)
target_link_libraries(step1134_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1135_test tests/step1135_test.cpp)
target_include_directories(step1135_test PRIVATE src)
target_link_libraries(step1135_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1136_test tests/step1136_test.cpp)
target_include_directories(step1136_test PRIVATE src)
target_link_libraries(step1136_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1137_test tests/step1137_test.cpp)
target_include_directories(step1137_test PRIVATE src)
target_link_libraries(step1137_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1138_test tests/step1138_test.cpp)
target_include_directories(step1138_test PRIVATE src)
target_link_libraries(step1138_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1139_test tests/step1139_test.cpp)
target_include_directories(step1139_test PRIVATE src)
target_link_libraries(step1139_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1140_test tests/step1140_test.cpp)
target_include_directories(step1140_test PRIVATE src)
target_link_libraries(step1140_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1141_test tests/step1141_test.cpp)
target_include_directories(step1141_test PRIVATE src)
target_link_libraries(step1141_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1142_test tests/step1142_test.cpp)
target_include_directories(step1142_test PRIVATE src)
target_link_libraries(step1142_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1143_test tests/step1143_test.cpp)
target_include_directories(step1143_test PRIVATE src)
target_link_libraries(step1143_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1144_test tests/step1144_test.cpp)
target_include_directories(step1144_test PRIVATE src)
target_link_libraries(step1144_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1145_test tests/step1145_test.cpp)
target_include_directories(step1145_test PRIVATE src)
target_link_libraries(step1145_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1146_test tests/step1146_test.cpp)
target_include_directories(step1146_test PRIVATE src)
target_link_libraries(step1146_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1147_test tests/step1147_test.cpp)
target_include_directories(step1147_test PRIVATE src)
target_link_libraries(step1147_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1148_test tests/step1148_test.cpp)
target_include_directories(step1148_test PRIVATE src)
target_link_libraries(step1148_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1149_test tests/step1149_test.cpp)
target_include_directories(step1149_test PRIVATE src)
target_link_libraries(step1149_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1150_test tests/step1150_test.cpp)
target_include_directories(step1150_test PRIVATE src)
target_link_libraries(step1150_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1151_test tests/step1151_test.cpp)
target_include_directories(step1151_test PRIVATE src)
target_link_libraries(step1151_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1152_test tests/step1152_test.cpp)
target_include_directories(step1152_test PRIVATE src)
target_link_libraries(step1152_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1153_test tests/step1153_test.cpp)
target_include_directories(step1153_test PRIVATE src)
target_link_libraries(step1153_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1154_test tests/step1154_test.cpp)
target_include_directories(step1154_test PRIVATE src)
target_link_libraries(step1154_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1155_test tests/step1155_test.cpp)
target_include_directories(step1155_test PRIVATE src)
target_link_libraries(step1155_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1156_test tests/step1156_test.cpp)
target_include_directories(step1156_test PRIVATE src)
target_link_libraries(step1156_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1157_test tests/step1157_test.cpp)
target_include_directories(step1157_test PRIVATE src)
target_link_libraries(step1157_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)

add_executable(step1158_test tests/step1158_test.cpp)
target_include_directories(step1158_test PRIVATE src)
target_link_libraries(step1158_test PRIVATE nlohmann_json::nlohmann_json unofficial::tree-sitter::tree-sitter tree_sitter_python tree_sitter_cpp tree_sitter_elisp tree_sitter_javascript tree_sitter_typescript tree_sitter_java tree_sitter_rust tree_sitter_go)
