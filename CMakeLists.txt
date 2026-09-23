cmake_minimum_required(VERSION 3.22)

project(SystemR
    VERSION 1.0.0
    DESCRIPTION "System R-style query engine over an LMDB-backed RSS"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

# LMDB is our Research Storage System's page/B-tree engine. It has no CMake
# build of its own, so we vendor its two source files directly.
include(FetchContent)
FetchContent_Declare(
    lmdb
    GIT_REPOSITORY https://github.com/LMDB/lmdb.git
    GIT_TAG LMDB_0.9.36
)
FetchContent_MakeAvailable(lmdb)

add_library(lmdb STATIC
    "${lmdb_SOURCE_DIR}/libraries/liblmdb/mdb.c"
    "${lmdb_SOURCE_DIR}/libraries/liblmdb/midl.c"
)
target_include_directories(lmdb PUBLIC "${lmdb_SOURCE_DIR}/libraries/liblmdb")
target_link_libraries(lmdb PUBLIC Threads::Threads)

file(GLOB_RECURSE QUERY_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(query_engine ${QUERY_ENGINE_SOURCES})
target_include_directories(query_engine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(query_engine
    PUBLIC
        lmdb
)

function(add_query_engine_app target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE query_engine)
endfunction()

add_query_engine_app(systemr apps/example/main.cpp)

include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)

    add_executable(query_engine_tests
        test/unit/datatypes/column_vector.cpp
        test/unit/datatypes/row_codec.cpp
        test/unit/storage/data_table.cpp
        test/unit/optimizer/access_path.cpp
    )
    target_link_libraries(query_engine_tests
        PRIVATE
            query_engine
            GTest::gtest_main
    )

    gtest_discover_tests(query_engine_tests)
endif()
