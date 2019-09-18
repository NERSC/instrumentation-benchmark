# include guard
include_guard(DIRECTORY)

##########################################################################################
#
#        Handles the build settings
#
##########################################################################################

include(GNUInstallDirs)
include(Compilers)

add_library(instrument-compile-options INTERFACE)
add_library(instrument-arch            INTERFACE)
add_library(instrument-enabled         INTERFACE)

#----------------------------------------------------------------------------------------#
# set the compiler flags
#
add_target_flag_if_avail(instrument-compile-options
    "-W"
    "-Wall"
    "-Wno-unknown-pragmas"
    "-Wno-ignored-attributes"
    "-Wno-attributes"
    "-Wno-unused-local-typedefs"
    "-faligned-new")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_target_flag(instrument-compile-options "-DDEBUG")
endif()

add_target_flag(instrument-enabled "-DUSE_INST")

#----------------------------------------------------------------------------------------#
# user customization
#
#   adds environment defined CFLAGS, CXXFLAGS, C_FLAGS, or CXX_FLAGS
#
add_user_flags(instrument-compile-options "C")
add_user_flags(instrument-compile-options "CXX")

#----------------------------------------------------------------------------------------#
# architecture optimizations
#
if(USE_ARCH)
    find_package(CpuArch)

    if(CpuArch_FOUND)
        foreach(_ARCH ${CpuArch_FEATURES})
            # intel compiler
            if(CMAKE_C_COMPILER_IS_INTEL OR CMAKE_CXX_COMPILER_IS_INTEL)
                add_target_flag_if_avail(instrument-arch "-x${_ARCH}")
            endif()
            # non-intel compilers
            if(NOT CMAKE_C_COMPILER_IS_INTEL OR NOT CMAKE_CXX_COMPILER_IS_INTEL)
                add_target_flag_if_avail(instrument-arch "-m${_ARCH}")
            endif()
        endforeach()
    endif()
    target_link_libraries(instrument-compile-options INTERFACE instrument-arch)
endif()
