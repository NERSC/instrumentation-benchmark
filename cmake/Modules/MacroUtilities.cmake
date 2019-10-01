# include guard
include_guard(DIRECTORY)

# MacroUtilities - useful macros and functions for generic tasks
#

cmake_policy(PUSH)
cmake_policy(SET CMP0054 NEW)

include(CMakeDependentOption)
include(CMakeParseArguments)


#----------------------------------------------------------------------------------------#
# macro CHECKOUT_GIT_SUBMODULE()
#
#   Run "git submodule update" if a file in a submodule does not exist
#
#   ARGS:
#       RECURSIVE (option) -- add "--recursive" flag
#       RELATIVE_PATH (one value) -- typically the relative path to submodule
#                                    from PROJECT_SOURCE_DIR
#       WORKING_DIRECTORY (one value) -- (default: PROJECT_SOURCE_DIR)
#       TEST_FILE (one value) -- file to check for (default: CMakeLists.txt)
#       ADDITIONAL_CMDS (many value) -- any addition commands to pass
#
MACRO(CHECKOUT_GIT_SUBMODULE)
    # parse args
    cmake_parse_arguments(
        CHECKOUT
        "RECURSIVE"
        "RELATIVE_PATH;WORKING_DIRECTORY;TEST_FILE"
        "ADDITIONAL_CMDS"
        ${ARGN})

    if(NOT CHECKOUT_WORKING_DIRECTORY)
        set(CHECKOUT_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif(NOT CHECKOUT_WORKING_DIRECTORY)

    if(NOT CHECKOUT_TEST_FILE)
        set(CHECKOUT_TEST_FILE "CMakeLists.txt")
    endif(NOT CHECKOUT_TEST_FILE)

    set(_DIR "${CHECKOUT_WORKING_DIRECTORY}/${CHECKOUT_RELATIVE_PATH}")
    # ensure the (possibly empty) directory exists
    if(NOT EXISTS "${_DIR}")
        message(FATAL_ERROR "submodule directory does not exist")
    endif(NOT EXISTS "${_DIR}")

    # if this file exists --> project has been checked out
    # if not exists --> not been checked out
    set(_TEST_FILE "${_DIR}/${CHECKOUT_TEST_FILE}")

    # if the module has not been checked out
    if(NOT EXISTS "${_TEST_FILE}")
        find_package(Git REQUIRED)

        set(_RECURSE )
        if(CHECKOUT_RECURSIVE)
            set(_RECURSE --recursive)
        endif(CHECKOUT_RECURSIVE)

        # perform the checkout
        execute_process(
            COMMAND
                ${GIT_EXECUTABLE} submodule update --init ${_RECURSE}
                    ${CHECKOUT_ADDITIONAL_CMDS} ${CHECKOUT_RELATIVE_PATH}
            WORKING_DIRECTORY
                ${CHECKOUT_WORKING_DIRECTORY}
            RESULT_VARIABLE RET)

        # check the return code
        if(RET GREATER 0)
            set(_CMD "${GIT_EXECUTABLE} submodule update --init ${_RECURSE}
                ${CHECKOUT_ADDITIONAL_CMDS} ${CHECKOUT_RELATIVE_PATH}")
            message(STATUS "macro(CHECKOUT_SUBMODULE) failed.")
            message(FATAL_ERROR "Command: \"${_CMD}\"")
        endif()

    endif()

ENDMACRO()


# check a required variable was defined
FUNCTION(CHECK_REQ_VAR _VARNAME)
    if(NOT DEFINED MODULE_${_VARNAME} OR "${MODULE_${_VARNAME}}" STREQUAL "")
        message(FATAL_ERROR "DEFINE_SUBMODULE requires the variable: '${_VARNAME}'")
    endif()
ENDFUNCTION()


# set/add a value to the cache
FUNCTION(SET_CACHE_VAR _MOD _NAME _VALUE _DESCRIPT)
    if("APPEND" IN_LIST ARGN AND DEFINED MODULE_${_MOD}_${_NAME})
        if(NOT "${_VALUE}" IN_LIST MODULE_${_MOD}_${_NAME})
            set(MODULE_${_MOD}_${_NAME} "MODULE_${_MOD}_${_NAME};${_VALUE}" CACHE STRING "${_DESCRIPT}" FORCE)
        endif()
    else()
        # use FORCE to always override
        set(MODULE_${_MOD}_${_NAME} "${_VALUE}" CACHE STRING "${_DESCRIPT}" FORCE)
    endif()
ENDFUNCTION()


# get a module value from the cache
FUNCTION(GET_CACHE_VAR _VAR _MOD _NAME)
    set(${_VAR} ${MODULE_${_MOD}_${_NAME}} PARENT_SCOPE)
ENDFUNCTION()


FUNCTION(DEFINE_SUBMODULE)
    # parse args
    cmake_parse_arguments(
        MODULE
        "REFERENCE"
        "NAME;HEADER_FILE;INTERFACE_LIBRARY;LANGUAGE;LINKER_LANGUAGE"
        "EXTRA_LANGUAGES"
        ${ARGN})

    # check required variables
    check_req_var(NAME)
    check_req_var(HEADER_FILE)
    check_req_var(INTERFACE_LIBRARY)
    check_req_var(LANGUAGE)

    # ensure no dashes in module name
    string(REPLACE "-" "_" MODULE_NAME "${MODULE_NAME}")

    # provide default linker language
    if(NOT MODULE_LINKER_LANGUAGE)
        set(MODULE_LINKER_LANGUAGE ${MODULE_LANGUAGE})
    endif()

    # check header language
    set(_HEADER)
    if(NOT MODULE_REFERENCE)
        if(EXISTS ${PROJECT_SOURCE_DIR}/include/user/${MODULE_HEADER_FILE})
            set(_HEADER user/${MODULE_HEADER_FILE})
        elseif(EXISTS ${MODULE_HEADER_FILE})
            # get the absolute path
            get_filename_component(_HEADER_FPATH "${MODULE_HEADER_FILE}" REALPATH)
            # get the relative path from include -- intended to fail if not in correct directory
            string(REPLACE "${PROJECT_SOURCE_DIR}/include/user/" "" MODULE_HEADER_FILE "${_HEADER_FPATH}")
            set(_HEADER user/${MODULE_HEADER_FILE})
        endif()
    else()
        set(_HEADER ${MODULE_HEADER_FILE})
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/include/${_HEADER}" OR "${_HEADER}" STREQUAL ""
       OR IS_DIRECTORY "${PROJECT_SOURCE_DIR}/include/${_HEADER}")
        message(FATAL_ERROR "Error locating header file: \"${MODULE_HEADER_FILE}\"")
    endif()

    # check interface library exists
    if(NOT TARGET ${MODULE_INTERFACE_LIBRARY})
        message(FATAL_ERROR "Interface library '${MODULE_INTERFACE_LIBRARY}' does not exist!")
    endif()

    set(_VALID_LANGUAGES C CXX CUDA Fortran)

    # check language
    if(NOT "${MODULE_LANGUAGE}" IN_LIST _VALID_LANGUAGES)
        message(FATAL_ERROR "LANGUAGE '${MODULE_LANGUAGE}' is not one of: ${_VALID_LANGUAGES}")
    endif()

    # check linker language
    if(NOT "${MODULE_LINKER_LANGUAGE}" IN_LIST _VALID_LANGUAGES)
        message(FATAL_ERROR "LINKER_LANGUAGE '${MODULE_LINKER_LANGUAGE}' is not one of: ${_VALID_LANGUAGES}")
    endif()

    # assemble cache variables
    set(_NAME ${MODULE_NAME})
    set_property(GLOBAL APPEND PROPERTY INST_MODULE_NAMES "${MODULE_NAME}")
    set_cache_var(${_NAME} HEADER_FILE       "${_HEADER}"                   "Header file for ${_NAME}")
    set_cache_var(${_NAME} INTERFACE_LIBRARY "${MODULE_INTERFACE_LIBRARY}"  "Interface library for ${_NAME}")
    set_cache_var(${_NAME} LANGUAGE          "${MODULE_LANGUAGE}"           "Language for ${_NAME}")
    set_cache_var(${_NAME} LINKER_LANGUAGE   "${MODULE_LINKER_LANGUAGE}"    "Linker Language for ${_NAME}")
    set_cache_var(${_NAME} IS_REFERENCE      "${MODULE_REFERENCE}"          "${_NAME} is reference target")

    foreach(_EXTRA ${MODULE_EXTRA_LANGUAGES})
        set_cache_var(${_NAME} EXTRA_LANGUAGES "${_EXTRA}" "Additional languages in ${_NAME}")
    endforeach()

ENDFUNCTION()

cmake_policy(POP)
