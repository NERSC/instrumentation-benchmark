
# common list of components
set(_COMPONENTS headers c cxx compile-options arch papi)
set(TIMEMORY_COMPONENTS "${_COMPONENTS}" CACHE STRING "timemory components")

option(ENABLE_CALIPER "Enable Caliper benchmarking" OFF)
option(ENABLE_TAU "Enable TAU benchmarking" OFF)

set(timemory_FIND_COMPONENTS_INTERFACE timemory-config)
find_package(timemory REQUIRED COMPONENTS ${TIMEMORY_COMPONENTS})

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
    NAME                stack_template
    LANGUAGE            CXX
    HEADER_FILE         timemory_cxx_templates.hpp
    INTERFACE_LIBRARY   timemory-config
)

define_submodule(
    NAME                heap_template
    LANGUAGE            CXX
    HEADER_FILE         timemory_cxx_templates_pointer.hpp
    INTERFACE_LIBRARY   timemory-config
)

define_submodule(
    NAME                functional_template
    LANGUAGE            CXX
    HEADER_FILE         timemory_cxx_bundle.hpp
    INTERFACE_LIBRARY   timemory-config
)

if(ENABLE_CALIPER)

    find_package(caliper REQUIRED)

    add_library(caliper-config INTERFACE)
    target_link_libraries(caliper-config INTERFACE caliper)
    target_include_directories(caliper-config INTERFACE ${caliper_INCLUDE_DIR})

    define_submodule(
        NAME                caliper_marker
        LANGUAGE            CXX
        HEADER_FILE         caliper_thread_scope.h
        INTERFACE_LIBRARY   caliper-config
        EXTRA_LANGUAGES     C
    )

endif()

if(ENABLE_TAU)

    add_library(tau INTERFACE)
    if(TARGET timemory-tau)
        target_link_libraries(tau INTERFACE timemory-tau)
    else()
        find_package(TAU REQUIRED)
        target_link_libraries(tau INTERFACE ${TAU_LIBRARIES})
        target_include_directories(tau INTERFACE ${TAU_INCLUDE_DIRS})
    endif()

    define_submodule(
        NAME                tau_marker
        LANGUAGE            CXX
        HEADER_FILE         tau_marker.h
        INTERFACE_LIBRARY   tau
        EXTRA_LANGUAGES     C
    )

endif()
