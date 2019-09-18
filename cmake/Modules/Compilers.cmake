# include guard
include_guard(DIRECTORY)

##########################################################################################
#
#        Compilers
#
##########################################################################################
#
#   sets (cached):
#
#       CMAKE_C_COMPILER_IS_<TYPE>
#       CMAKE_CXX_COMPILER_IS_<TYPE>
#
#   where TYPE is:
#       - GNU
#       - CLANG
#       - INTEL
#       - INTEL_ICC
#       - INTEL_ICPC
#       - PGI
#       - XLC
#       - HP_ACC
#       - MIPS
#       - MSVC
#

include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)

include(MacroUtilities)

#----------------------------------------------------------------------------------------#
# macro converting string to list
#----------------------------------------------------------------------------------------#
macro(to_list _VAR _STR)
    STRING(REPLACE "  " " " ${_VAR} "${_STR}")
    STRING(REPLACE " " ";" ${_VAR} "${_STR}")
endmacro(to_list _VAR _STR)


#----------------------------------------------------------------------------------------#
# macro converting string to list
#----------------------------------------------------------------------------------------#
macro(to_string _VAR _STR)
    STRING(REPLACE ";" " " ${_VAR} "${_STR}")
endmacro(to_string _VAR _STR)


#----------------------------------------------------------------------------------------#
#   Macro to add to string
#----------------------------------------------------------------------------------------#
macro(add _VAR _FLAG)
    if(NOT "${_FLAG}" STREQUAL "")
        if("${${_VAR}}" STREQUAL "")
            set(${_VAR} "${_FLAG}")
        else()
            set(${_VAR} "${${_VAR}} ${_FLAG}")
        endif()
    endif()
endmacro()


##########################################################################################
#
#                               C compiler flags
#
##########################################################################################


#----------------------------------------------------------------------------------------#
# add C flag to target
#----------------------------------------------------------------------------------------#
macro(ADD_TARGET_C_FLAG _TARG)
    target_compile_options(${_TARG} INTERFACE $<$<COMPILE_LANGUAGE:C>:${ARGN}>)
endmacro()


#----------------------------------------------------------------------------------------#
# check C flag
#----------------------------------------------------------------------------------------#
macro(ADD_C_FLAG_IF_AVAIL _TARG FLAG)
    if(NOT "${FLAG}" STREQUAL "")
        string(REGEX REPLACE "^/" "c_${_TARG}_" FLAG_NAME "${FLAG}")
        string(REGEX REPLACE "^-" "c_${_TARG}_" FLAG_NAME "${FLAG}")
        string(REPLACE "-" "_" FLAG_NAME "${FLAG_NAME}")
        string(REPLACE " " "_" FLAG_NAME "${FLAG_NAME}")
        string(REPLACE "=" "_" FLAG_NAME "${FLAG_NAME}")
        check_c_compiler_flag("${FLAG}" ${FLAG_NAME})
        if(${FLAG_NAME})
            add_target_c_flag(${_TARG} ${FLAG})
        endif()
    endif()
endmacro()


#----------------------------------------------------------------------------------------#
# add C flag to target -- USE THIS FUNCTION
#----------------------------------------------------------------------------------------#
macro(ADD_TARGET_C_FLAG_IF_AVAIL _TARG)
    foreach(_FLAG ${ARGN})
        add_c_flag_if_avail(${_TARG} ${_FLAG})
    endforeach()
endmacro()


##########################################################################################
#
#                                   CXX compiler flags
#
##########################################################################################


#----------------------------------------------------------------------------------------#
# add CXX flag to target
#----------------------------------------------------------------------------------------#
macro(ADD_TARGET_CXX_FLAG _TARG)
    target_compile_options(${_TARG} INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${ARGN}>)
endmacro()


#----------------------------------------------------------------------------------------#
# check CXX flag
#----------------------------------------------------------------------------------------#
macro(ADD_CXX_FLAG_IF_AVAIL _TARG FLAG)
    if(NOT "${FLAG}" STREQUAL "")
        string(REGEX REPLACE "^/" "cxx_${_TARG}_" FLAG_NAME "${FLAG}")
        string(REGEX REPLACE "^-" "cxx_${_TARG}_" FLAG_NAME "${FLAG}")
        string(REPLACE "-" "_" FLAG_NAME "${FLAG_NAME}")
        string(REPLACE " " "_" FLAG_NAME "${FLAG_NAME}")
        string(REPLACE "=" "_" FLAG_NAME "${FLAG_NAME}")
        check_cxx_compiler_flag("${FLAG}" ${FLAG_NAME})
        if(${FLAG_NAME})
            add_target_cxx_flag(${_TARG} ${FLAG})
        endif()
    endif()
endmacro()


#----------------------------------------------------------------------------------------#
# add CXX flag to target
#----------------------------------------------------------------------------------------#
macro(ADD_TARGET_CXX_FLAG_IF_AVAIL _TARG)
    foreach(_FLAG ${ARGN})
        add_cxx_flag_if_avail(${_TARG} ${_FLAG})
    endforeach()
endmacro()


##########################################################################################
#
#                                       Common
#
##########################################################################################


#----------------------------------------------------------------------------------------#
# add C and CXX flag w/o checking
#----------------------------------------------------------------------------------------#
macro(ADD_TARGET_FLAG _TARG)
    foreach(_ARG ${ARGN})
        ADD_TARGET_C_FLAG(${_TARG} ${_ARG})
        ADD_TARGET_CXX_FLAG(${_TARG} ${_ARG})
    endforeach()
endmacro()


#----------------------------------------------------------------------------------------#
# check C and CXX flag
#----------------------------------------------------------------------------------------#
macro(ADD_TARGET_FLAG_IF_AVAIL _TARG)
    foreach(_ARG ${ARGN})
        ADD_TARGET_C_FLAG_IF_AVAIL(${_TARG} ${_ARG})
        ADD_TARGET_CXX_FLAG_IF_AVAIL(${_TARG} ${_ARG})
    endforeach()
endmacro()


#----------------------------------------------------------------------------------------#
# add to any language
#----------------------------------------------------------------------------------------#
function(ADD_USER_FLAGS _TARGET _LANGUAGE)

    set(_FLAGS ${${_LANGUAGE}FLAGS} $ENV{${_LANGUAGE}FLAGS}
        ${${_LANGUAGE}_FLAGS} $ENV{${_LANGUAGE}_FLAGS})

    string(REPLACE " " ";" _FLAGS "${_FLAGS}")

    set(${PROJECT_NAME}_${_LANGUAGE}_FLAGS
        ${${PROJECT_NAME}_${_LANGUAGE}_FLAGS} ${_FLAGS} PARENT_SCOPE)

    set(${PROJECT_NAME}_${_LANGUAGE}_COMPILE_OPTIONS
        ${${PROJECT_NAME}_${_LANGUAGE}_COMPILE_OPTIONS} ${_FLAGS} PARENT_SCOPE)

    target_compile_options(${_TARGET} INTERFACE
        $<$<COMPILE_LANGUAGE:${_LANGUAGE}>:${_FLAGS}>)
endfunction()


#----------------------------------------------------------------------------------------#
# determine compiler types for each language
#----------------------------------------------------------------------------------------#
foreach(LANG C CXX)

    macro(SET_COMPILER_VAR VAR _BOOL)
        set(CMAKE_${LANG}_COMPILER_IS_${VAR} ${_BOOL} CACHE BOOL
            "CMake ${LANG} compiler identification (${VAR})")
        mark_as_advanced(CMAKE_${LANG}_COMPILER_IS_${VAR})
    endmacro()

    if(("${LANG}" STREQUAL "C" AND CMAKE_COMPILER_IS_GNUCC)
        OR
       ("${LANG}" STREQUAL "CXX" AND CMAKE_COMPILER_IS_GNUCXX))

        # GNU compiler
        SET_COMPILER_VAR(       GNU                 ON)

    elseif(CMAKE_${LANG}_COMPILER MATCHES "icc.*")

        # Intel icc compiler
        SET_COMPILER_VAR(       INTEL               ON)
        SET_COMPILER_VAR(       INTEL_ICC           ON)

    elseif(CMAKE_${LANG}_COMPILER MATCHES "icpc.*")

        # Intel icpc compiler
        SET_COMPILER_VAR(       INTEL               ON)
        SET_COMPILER_VAR(       INTEL_ICPC          ON)

    elseif(CMAKE_${LANG}_COMPILER_ID MATCHES "Clang" OR
           CMAKE_${LANG}_COMPILER_ID MATCHES "AppleClang")

        # Clang/LLVM compiler
        SET_COMPILER_VAR(       CLANG               ON)

    elseif(CMAKE_${LANG}_COMPILER_ID MATCHES "PGI")

        # PGI compiler
        SET_COMPILER_VAR(       PGI                 ON)

    elseif(CMAKE_${LANG}_COMPILER MATCHES "xlC" AND UNIX)

        # IBM xlC compiler
        SET_COMPILER_VAR(       XLC                 ON)

    elseif(CMAKE_${LANG}_COMPILER MATCHES "aCC" AND UNIX)

        # HP aC++ compiler
        SET_COMPILER_VAR(       HP_ACC              ON)

    elseif(CMAKE_${LANG}_COMPILER MATCHES "CC" AND
           CMAKE_SYSTEM_NAME MATCHES "IRIX" AND UNIX)

        # IRIX MIPSpro CC Compiler
        SET_COMPILER_VAR(       MIPS                ON)

    elseif(CMAKE_${LANG}_COMPILER_ID MATCHES "Intel")

        SET_COMPILER_VAR(       INTEL               ON)

        set(CTYPE ICC)
        if("${LANG}" STREQUAL "CXX")
            set(CTYPE ICPC)
        endif()

        SET_COMPILER_VAR(       INTEL_${CTYPE}      ON)

    elseif(CMAKE_${LANG}_COMPILER MATCHES "MSVC")

        # Windows Visual Studio compiler
        SET_COMPILER_VAR(       MSVC                ON)

    endif()

    # set other to no
    foreach(TYPE GNU INTEL INTEL_ICC INTEL_ICPC CLANG PGI XLC HP_ACC MIPS MSVC)
        if(NOT ${CMAKE_${LANG}_COMPILER_IS_${TYPE}})
            SET_COMPILER_VAR(${TYPE} OFF)
        endif()
    endforeach()

endforeach()
