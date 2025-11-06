cmake_minimum_required(VERSION 3.16)

project(TIPE-BDD LANGUAGES CXX)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/googletest)

add_custom_target(memcheck
    COMMAND valgrind --leak-check=full --track-origins=yes ./src/TIPE-BDD_run
    DEPENDS ${PROJECT_NAME}
)