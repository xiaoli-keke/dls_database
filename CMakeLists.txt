cmake_minimum_required(VERSION 2.8)
project(ddatabase)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)

set(LIB_SRC
    src/coding.cc
    src/database.cc
    src/memtable.cc
    src/disk.cc
    src/block.cc
    src/blockbuilder.cc
    src/footer.cc
    src/tablebuilder.cc
    src/tablereader.cc
    )

add_library(ddatabse SHARED ${LIB_SRC})

set(LIB_LIB
    ddatabse
    pthread
    )

add_executable(test01 tests/test01.cc)
add_dependencies(test01 ddatabse)
target_link_libraries(test01 ${LIB_LIB})

add_executable(test02 tests/test02.cc)
add_dependencies(test02 ddatabse)
target_link_libraries(test02 ${LIB_LIB})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)