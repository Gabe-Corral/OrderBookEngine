cmake_minimum_required(VERSION 3.20)

project(order_book_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ob_core
    src/util/logger.cpp
    src/book/order_book.cpp
)

target_include_directories(ob_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(ob_core
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
    )
endif()

add_executable(order_book
    src/main.cpp
)

target_link_libraries(order_book
    PRIVATE ob_core
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(order_book
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
    )
endif()

option(OB_ENABLE_TESTS "Enable tests" ON)

if (OB_ENABLE_TESTS)
	enable_testing()
    add_subdirectory(tests)
endif()
