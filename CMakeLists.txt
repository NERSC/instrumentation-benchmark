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
option(BUILD_SHARED_LIBS "Enable building shared libraries" ON)

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
#   import pybind11
#----------------------------------------------------------------------------------------#

if(NOT TARGET pybind11)
    checkout_git_submodule(RECURSIVE RELATIVE_PATH external/pybind11
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    add_subdirectory(external/pybind11)
endif()

#----------------------------------------------------------------------------------------#
#   include-directories
#----------------------------------------------------------------------------------------#

add_library(instrument-headers INTERFACE)
target_include_directories(instrument-headers INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_libraries(instrument-headers INTERFACE instrument-compile-options)

if(USE_ARCH)
    target_link_libraries(instrument-headers INTERFACE instrument-arch)
endif()

# use GLOB so we can easily build more tests without editing
file(GLOB C_SOURCES     ${PROJECT_SOURCE_DIR}/source/*.c)
file(GLOB CXX_SOURCES   ${PROJECT_SOURCE_DIR}/source/*.cpp)
file(GLOB PYC_SOURCES   ${PROJECT_SOURCE_DIR}/source/python/*.cpp)

# this is just so the headers show up in IDEs
file(GLOB C_HEADERS     ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB CXX_HEADERS   ${PROJECT_SOURCE_DIR}/include/*.hpp)

# interface libraries that provide pre-processor for python submodule calls
get_property(ENABLED_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach(_LANG ${ENABLED_LANGUAGES})
    add_library(${_LANG}-language INTERFACE)
    target_compile_definitions(${_LANG}-language INTERFACE USE_${_LANG})
endforeach()

#----------------------------------------------------------------------------------------#
#   create libraries
#----------------------------------------------------------------------------------------#
add_library(fallback-config INTERFACE)

define_submodule(
    REFERENCE
    NAME                baseline
    LANGUAGE            CXX
    HEADER_FILE         fallback_inst.h
    INTERFACE_LIBRARY   fallback-config
    EXTRA_LANGUAGES     C
)

# define_submodule(
#     NAME                dormant
#     LANGUAGE            CXX
#     HEADER_FILE         ../fallback_inst.h
#     INTERFACE_LIBRARY   fallback-config
#     EXTRA_LANGUAGES     C
# )

# get all the modules
get_property(INST_MODULE_NAMES GLOBAL PROPERTY INST_MODULE_NAMES)

message(STATUS "Modules: ${INST_MODULE_NAMES}")

# for python -- @ONLY variables
set(INST_SUBMODULE_LIST)
set(INST_SUBMODULE_IMPORT)

# create the libraries containing the compiled tests
foreach(_MODULE ${INST_MODULE_NAMES})
    # get the cache variables for the module
    get_cache_var(_LANG         ${_MODULE} LANGUAGE)
    get_cache_var(_HEADER_FILE  ${_MODULE} HEADER_FILE)
    get_cache_var(_LINK_LANG    ${_MODULE} LINKER_LANGUAGE)
    get_cache_var(_INTERFACE    ${_MODULE} INTERFACE_LIBRARY)
    get_cache_var(_EXTRA_LANGS  ${_MODULE} EXTRA_LANGUAGES)
    get_cache_var(_IS_REF       ${_MODULE} IS_REFERENCE)

    string(REPLACE "_" "-" _TARGET_MODULE "${_MODULE}")

    # set the interface libraries to link to
    set(_INTERFACE_LIBS ${_INTERFACE})
    # add the language-specific interface libraries
    foreach(_ENTRY ${_LANG} ${_EXTRA_LANGS})
        list(APPEND _INTERFACE_LIBS ${_ENTRY}-language)
    endforeach()

    if(TARGET global-config)
        list(APPEND _INTERFACE_LIBS global-config)
    endif()

    if(NOT _IS_REF)
        list(APPEND _INTERFACE_LIBS instrument-enabled)
    endif()

    list(REMOVE_DUPLICATES _INTERFACE_LIBS)
    message(STATUS "${_MODULE} interface libraries: ${_INTERFACE_LIBS}")

    # output path of libraries
    set(SUBMODULE_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/instrument_benchmark/${_MODULE})

    # for python
    list(APPEND INST_SUBMODULE_LIST ${_MODULE})
    list(APPEND INST_SUBMODULE_IMPORT "from . import ${_MODULE}")

    # sources to build
    set(_TARGET_SOURCES)
    set(_TARGET_HEADERS ${${_LANG}_HEADERS} ${PROJECT_SOURCE_DIR}/include/${_HEADER_FILE})

    # @ONLY variable
    set(SUBMODULE_HEADER_FILE ${_HEADER_FILE})

    set(_LANG_SOURCES)
    foreach(_ENTRY ${_LANG} ${_EXTRA_LANGS})
        list(APPEND _LANG_SOURCES ${${_ENTRY}_SOURCES})
    endforeach()
    # message(STATUS "[${_MODULE}] sources: ${_SOURCES}")

    # configure_file for all language sources
    foreach(_SOURCE ${_LANG_SOURCES})
        # get the realpath in source directory
        get_filename_component(_BINARY ${_SOURCE} REALPATH)
        # get the name of the file
        get_filename_component(_FILENAME  ${_SOURCE} NAME_WE)
        # convert to to binary directory
        string(REPLACE "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" _BINARY "${_BINARY}")
        # convert to instrumentation file
        string(REPLACE "${_FILENAME}" "${_FILENAME}_${_MODULE}" _BINARY "${_BINARY}")
        # configure
        configure_file(${_SOURCE} ${_BINARY} @ONLY)
        # add the source
        list(APPEND _TARGET_SOURCES ${_BINARY})
    endforeach()

    # name of the submodule -- @ONLY variable
    set(SUBMODULE_LIBRARY_NAME libpy${_MODULE})

    # create library
    add_library(inst-bench-${_TARGET_MODULE} SHARED ${_TARGET_SOURCES} ${_TARGET_HEADERS})
    target_link_libraries(inst-bench-${_TARGET_MODULE}
        PUBLIC instrument-headers ${_INTERFACE_LIBS})
    set_target_properties(inst-bench-${_TARGET_MODULE} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${SUBMODULE_OUTPUT_PATH}
        LIBRARY_OUTPUT_DIRECTORY ${SUBMODULE_OUTPUT_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${SUBMODULE_OUTPUT_PATH}
        OUTPUT_NAME              ${_MODULE})

    # sources to build python interface from
    set(_PYTARG_SOURCES)
    # configure_file for all language sources
    foreach(_SOURCE ${PYC_SOURCES})
        # get the realpath in source directory
        get_filename_component(_BINARY ${_SOURCE} REALPATH)
        # get the name of the file
        get_filename_component(_PYNAME  ${_SOURCE} NAME_WE)
        # convert to to binary directory
        string(REPLACE "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" _BINARY "${_BINARY}")
        # convert to instrumentation file
        string(REPLACE "${_PYNAME}" "libpy${_MODULE}" _BINARY "${_BINARY}")
        #message(STATUS "[${_TARGET_MODULE}]> configuring ${_SOURCE} -> ${_BINARY}")
        # configure
        configure_file(${_SOURCE} ${_BINARY} @ONLY)
        # add the source
        list(APPEND _PYTARG_SOURCES ${_BINARY})
    endforeach()

    pybind11_add_module(py-inst-bench-${_TARGET_MODULE} ${_PYTARG_SOURCES})
    target_link_libraries(py-inst-bench-${_TARGET_MODULE} PRIVATE inst-bench-${_TARGET_MODULE} ${_INTERFACE_LIBS})
    set_target_properties(py-inst-bench-${_TARGET_MODULE} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${SUBMODULE_OUTPUT_PATH}
        LIBRARY_OUTPUT_DIRECTORY ${SUBMODULE_OUTPUT_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${SUBMODULE_OUTPUT_PATH}
        OUTPUT_NAME              ${SUBMODULE_LIBRARY_NAME}
        PREFIX                   "")

    # only one submodule can build these bindings
    if("${_MODULE}" STREQUAL "baseline")
        target_compile_definitions(py-inst-bench-${_TARGET_MODULE}
            PRIVATE BUILD_RUNTIME_DATA_BINDINGS)
    endif()

endforeach()


#----------------------------------------------------------------------------------------#
#   configure the python module for the build directory
#----------------------------------------------------------------------------------------#

# for (possible) later configuration of python additions by user
set(PYTHON_SUBMODULES ${INST_SUBMODULE_LIST})

# replace semi-colons with new-lines
string(REPLACE ";" "\n    " INST_SUBMODULE_IMPORT "${INST_SUBMODULE_IMPORT}")

configure_file(${PROJECT_SOURCE_DIR}/instrument_benchmark/__init__.py.in
    ${CMAKE_BINARY_DIR}/instrument_benchmark/__init__.py)


#----------------------------------------------------------------------------------------#
#   configure any remaining python files in source tree
#----------------------------------------------------------------------------------------#

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

    set(SUBMODULE_LIBRARY_NAME libpy${_SUBMODULE})
    configure_file(${PROJECT_SOURCE_DIR}/cmake/Templates/__init__.py
        ${BINARY_DIR}/__init__.py @ONLY)

endforeach()


#----------------------------------------------------------------------------------------#
#   copy over example file
#----------------------------------------------------------------------------------------#

configure_file(${PROJECT_SOURCE_DIR}/examples/execute.py
    ${CMAKE_BINARY_DIR}/execute.py COPYONLY)

configure_file(${PROJECT_SOURCE_DIR}/examples/execute.sh
    ${CMAKE_BINARY_DIR}/execute.sh COPYONLY)
