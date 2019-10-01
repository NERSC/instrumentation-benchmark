# include guard
include_guard(DIRECTORY)

##########################################################################################
#
#        Creates a 'format' target that runs clang-format
#
##########################################################################################


find_program(CLANG_FORMATTER
    NAMES
        clang-format-10.0
        clang-format-9.0
        clang-format-8.0
        clang-format-7.0
        clang-format-6.0
        clang-format)

if(CLANG_FORMATTER)

    file(GLOB_RECURSE format_headers
        ${PROJECT_SOURCE_DIR}/include/*.h
        ${PROJECT_SOURCE_DIR}/include/*.hpp)

    file(GLOB_RECURSE format_sources
        ${PROJECT_SOURCE_DIR}/source/*.c
        ${PROJECT_SOURCE_DIR}/source/*.cpp)

    set(FORMAT_NAME format)
    if(TARGET format)
        set(FORMAT_NAME format-instrumentation)
    endif()

    add_custom_target(${FORMAT_NAME}
        COMMAND ${CLANG_FORMATTER} -i ${format_headers}
        COMMAND ${CLANG_FORMATTER} -i ${format_sources}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Running '${CLANG_FORMATTER}'..."
        SOURCES ${format_headers} ${format_sources})
endif()
