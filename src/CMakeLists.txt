# Sources
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

# Library + executable
add_library(bdd_tipe_lib STATIC ${SOURCES})

target_include_directories(bdd_tipe_lib 
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bdd_tipe_lib PUBLIC tomlplusplus::tomlplusplus)

add_executable(bdd_tipe main.cpp)

target_link_libraries(bdd_tipe PUBLIC bdd_tipe_lib replxx)

target_compile_options(bdd_tipe PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(bdd_tipe PRIVATE -fsanitize=address)

# Enable gperftools profiling
option(USE_GPERFTOOLS "Use Google Performance Tools" ON)
if(USE_GPERFTOOLS)
    find_library(TCMALLOC_AND_PROFILER_LIB tcmalloc_and_profiler)
    find_library(PROFILER_LIB profiler)

    if(TCMALLOC_AND_PROFILER_LIB AND PROFILER_LIB)
        message(STATUS "Using gperftools tcmalloc_and_profiler: ${TCMALLOC_AND_PROFILER_LIB}")
        message(STATUS "Using gperftools profiler: ${PROFILER_LIB}")
        target_link_libraries(bdd_tipe PRIVATE ${TCMALLOC_AND_PROFILER_LIB} ${PROFILER_LIB})
    else()
        message(WARNING "gperftools libraries not found → Profiling disabled")
    endif()
endif()


