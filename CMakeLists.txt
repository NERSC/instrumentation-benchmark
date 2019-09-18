##########################################################################################
#       CMake settings
##########################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    set(MSG "")
    message(STATUS "Warning! Building from the source directory is not recommended")
    message(STATUS "If unintented, please remove 'CMakeCache.txt' and 'CMakeFiles'")
    message(STATUS "and build from a separate directory")
    message(WARNING "In-source build")
endif()

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0077 NEW)

#----------------------------------------------------------------------------------------#
#   project
#----------------------------------------------------------------------------------------#

project(Instrumentation-Benchmark LANGUAGES C CXX VERSION 0.0.1)

set(USER_CONFIG ${CMAKE_CURRENT_LIST_DIR}/cmake/UserSettings.cmake CACHE FILEPATH "User configuration file")
if(NOT EXISTS ${USER_CONFIG})
    set(MSG "\n")
    set(MSG "${MSG} Provide a user configuration file to include your project\n")
    set(MSG "${MSG} Recommended path: ${USER_CONFIG}\n")
    set(MSG "${MSG} This file should create a user-c-config and user-cxx-config INTERFACE libraries\n")
    message(FATAL_ERROR "${MSG} Missing '${USER_CONFIG}")
endif()

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/cmake         # user places UserSetting.cmake here
    ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules # private CMake modules
    ${CMAKE_CURRENT_LIST_DIR}/external/pybind11/tools
    ${CMAKE_MODULE_PATH})

set(CMAKE_C_STANDARD            11)
set(CMAKE_C_EXTENSIONS          OFF)
set(CMAKE_C_STANDARD_REQUIRED   ON)

set(CMAKE_CXX_STANDARD          11)
set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_ARCH "Enable architecture-specific flags" OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

#----------------------------------------------------------------------------------------#
# set the output directory (critical for Windows and Xcode)
#
foreach(_TYPE ARCHIVE LIBRARY RUNTIME)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/instrument_benchmark)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/instrument_benchmark)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/instrument_benchmark)
endforeach()


#----------------------------------------------------------------------------------------#
#   configuration
#----------------------------------------------------------------------------------------#

include(MacroUtilities)
# include(Options)
include(Compilers)
include(GNUInstallDirs)
include(BuildSettings)
include(ClangFormat)
include(CMakeParseArguments)
include(${USER_CONFIG})

#----------------------------------------------------------------------------------------#
#   include-directories
#----------------------------------------------------------------------------------------#

add_library(instrument-headers     INTERFACE)
target_include_directories(instrument-headers INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_libraries(instrument-headers INTERFACE instrument-compile-options)
if(USE_ARCH)
    target_link_libraries(instrument-headers INTERFACE instrument-arch)
endif()

# use GLOB so we can easily build more tests without editing
file(GLOB c_sources             ${PROJECT_SOURCE_DIR}/source/*.c)
file(GLOB cxx_sources           ${PROJECT_SOURCE_DIR}/source/*.cpp)

# this is just so the headers show up in IDEs
file(GLOB_RECURSE c_headers     ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE cxx_headers   ${PROJECT_SOURCE_DIR}/include/*.hpp)

#----------------------------------------------------------------------------------------#
#   create libraries
#----------------------------------------------------------------------------------------#

add_library(c-enable-instrument SHARED ${c_sources} ${c_headers})
target_link_libraries(c-enable-instrument PUBLIC
    instrument-headers instrument-enabled user-c-config)

add_library(cxx-enable-instrument SHARED ${cxx_sources} ${cxx_headers})
target_link_libraries(cxx-enable-instrument PUBLIC
    instrument-headers instrument-enabled user-cxx-config)

add_library(c-disable-instrument SHARED ${c_sources} ${c_headers})
target_link_libraries(c-disable-instrument PUBLIC
    instrument-headers)

add_library(cxx-disable-instrument SHARED ${cxx_sources} ${cxx_headers})
target_link_libraries(cxx-disable-instrument PUBLIC
    instrument-headers)

#----------------------------------------------------------------------------------------#
#   import pybind11
#----------------------------------------------------------------------------------------#

if(NOT TARGET pybind11)
    checkout_git_submodule(RECURSIVE RELATIVE_PATH external/pybind11
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    add_subdirectory(external/pybind11)
endif()

set(_PYBIND_MODULE_FILE ${PROJECT_SOURCE_DIR}/source/python/libinstrument_benchmark.cpp)
#----------------------------------------------------------------------------------------#
#   create python module that has everything enabled
#----------------------------------------------------------------------------------------#

pybind11_add_module(libinstrument_benchmark_enabled ${_PYBIND_MODULE_FILE})
target_link_libraries(libinstrument_benchmark_enabled PRIVATE
    c-enable-instrument cxx-enable-instrument)
target_compile_definitions(libinstrument_benchmark_enabled PRIVATE
    INST_MODULE_NAME=libinstrument_benchmark_enabled)

#----------------------------------------------------------------------------------------#
#   create python module that has everything disabled
#----------------------------------------------------------------------------------------#

pybind11_add_module(libinstrument_benchmark_disabled ${_PYBIND_MODULE_FILE})
target_link_libraries(libinstrument_benchmark_disabled PRIVATE
    c-disable-instrument cxx-disable-instrument)
target_compile_definitions(libinstrument_benchmark_disabled PRIVATE
    INST_MODULE_NAME=libinstrument_benchmark_disabled)

#----------------------------------------------------------------------------------------#
#   configure the python module for the build directory
#----------------------------------------------------------------------------------------#

configure_file(${PROJECT_SOURCE_DIR}/instrument_benchmark/__init__.py.in
    ${CMAKE_BINARY_DIR}/instrument_benchmark/__init__.py @ONLY)

# for later
set(PYTHON_SUBMODULES )
foreach(_SUBMODULE ${PYTHON_SUBMODULES})

    set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/instrument_benchmark/${_SUBMODULE})
    set(BINARY_DIR ${CMAKE_BINARY_DIR}/instrument_benchmark/${_SUBMODULE})

    if(EXISTS ${SOURCE_DIR} AND IS_DIRECTORY ${SOURCE_DIR})
        file(GLOB _FILES ${SOURCE_DIR}/*.py)
        foreach(_FILE ${_FILES})
            get_filename_component(_FNAME ${_FILE} NAME)
            configure_file(${SOURCE_DIR}/${_FNAME} ${BINARY_DIR}/${_FNAME} @ONLY)
        endforeach()
    else()
        # copy over the python file
        if(EXISTS ${SOURCE_DIR}/${_SUBMODULE}.py)
            configure_file(${SOURCE_DIR}/${_SUBMODULE}.py ${BINARY_DIR}/${_SUBMODULE}.py @ONLY)
        endif()
    endif()

endforeach()
