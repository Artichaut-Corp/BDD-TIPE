set(BINARY ${CMAKE_PROJECT_NAME})

# Sources
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

# Library + executable
add_library(${BINARY}_lib STATIC ${SOURCES})
add_executable(${BINARY}_run ${SOURCES})
target_link_libraries(${BINARY}_run PRIVATE ${BINARY}_lib)

# Enable gperftools profiling
option(USE_GPERFTOOLS "Use Google Performance Tools" ON)
if(USE_GPERFTOOLS)
    find_library(TCMALLOC_AND_PROFILER_LIB tcmalloc_and_profiler)
    find_library(PROFILER_LIB profiler)

    if(TCMALLOC_AND_PROFILER_LIB AND PROFILER_LIB)
        message(STATUS "Using gperftools tcmalloc_and_profiler: ${TCMALLOC_AND_PROFILER_LIB}")
        message(STATUS "Using gperftools profiler: ${PROFILER_LIB}")
        target_link_libraries(${BINARY}_run PRIVATE ${TCMALLOC_AND_PROFILER_LIB} ${PROFILER_LIB})
    else()
        message(WARNING "gperftools libraries not found → Profiling disabled")
    endif()
endif()
