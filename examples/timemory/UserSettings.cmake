
# common list of components
set(_COMPONENTS headers c cxx compile-options arch)
set(TIMEMORY_COMPONENTS "${_COMPONENTS}" CACHE STRING "timemory components")

option(ENABLE_CALIPER "Enable Caliper benchmarking" OFF)

find_package(timemory REQUIRED COMPONENTS ${TIMEMORY_COMPONENTS})

add_library(timemory-config INTERFACE)
target_link_libraries(timemory-config INTERFACE timemory)
if(CMAKE_CXX_COMPILER_IS_GNU)
    target_compile_options(timemory-config INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>)
endif()

include_directories(${timemory_INCLUDE_DIR})

define_submodule(
    NAME                library
    LANGUAGE            CXX
    HEADER_FILE         timemory_cxx_ptr_library.hpp
    INTERFACE_LIBRARY   timemory-config
    EXTRA_LANGUAGES     C
)

define_submodule(
    NAME                variadic_enum
    LANGUAGE            C
    HEADER_FILE         timemory_c_enum_library.h
    INTERFACE_LIBRARY   timemory-config
)

define_submodule(
    NAME                basic_marker
    LANGUAGE            CXX
    HEADER_FILE         timemory_cxx_templates.hpp
    INTERFACE_LIBRARY   timemory-config
)

define_submodule(
    NAME                basic_pointer
    LANGUAGE            CXX
    HEADER_FILE         timemory_cxx_templates_pointer.hpp
    INTERFACE_LIBRARY   timemory-config
)

if(ENABLE_CALIPER)

    define_submodule(
        NAME                cali_thread_scope
        LANGUAGE            CXX
        HEADER_FILE         caliper_thread_scope.h
        INTERFACE_LIBRARY   timemory-config timemory-caliper
        EXTRA_LANGUAGES     C
    )

    define_submodule(
        NAME                cali_process_scope
        LANGUAGE            CXX
        HEADER_FILE         caliper_process_scope.h
        INTERFACE_LIBRARY   timemory-config timemory-caliper
        EXTRA_LANGUAGES     C
    )

endif()
