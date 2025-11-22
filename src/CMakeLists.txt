# Sources
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

# Library + executable
add_library(bdd_tipe_lib STATIC ${SOURCES})

target_link_libraries(bdd_tipe_lib PUBLIC tomlplusplus::tomlplusplus)

add_executable(bdd_tipe ${SOURCES})

target_link_libraries(bdd_tipe PUBLIC bdd_tipe_lib tomlplusplus::tomlplusplus replxx)

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
