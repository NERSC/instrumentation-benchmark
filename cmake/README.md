# CMake User Configuration

Create a `UserSettings.cmake` in this folder or create a file with a different name and override the
`USER_CONFIG` CMake cache variable.

## Writing UserConfig.cmake

### Defining Submodule

Use the CMake function `DEFINE_SUBMODULE(...)` to create a submodule for testing.
Each invocation of this function creates a new Python submodule.

#### DEFINE_SUBMODULE Arguments

- `NAME`
    - name of the submodule
    - Number of Arguments : 1
- `LANGUAGE`
    - Options : C, CXX
    - Description
        - Determines which tests to compile and which options are available in the submodule
        - If the header supports more than one language, choose the linker language and add the other languages to `EXTRA_LANGUAGES`
    - Number of Arguments : 1
- `LINKER_LANGUAGE`
    - if a linker language different than `LANGUAGE` is required
    - Number of Arguments : 1
- `HEADER_FILE`
    - the name of file located in `include/user`
    - Number of Arguments : 1
- `INTERFACE_LIBRARY`
    - a CMake INTERFACE library that contains all the appropriate compile-definitions, build flags, include paths, and linked libraries to use the instrumentation API
    - Number of Arguments : 1
- `EXTRA_LANGUAGES`
    - if the submodule supports more languages than the one listed under `LANGUAGES`, list them here
    - Number of Arguments : > 1

### Example

```cmake
# common list of components
set(TIMEMORY_COMMON headers papi gotcha gperftools-cpu vector gotcha compile-options)
option(TEST_C_TIMEMORY "Enable timemory C tests" ON)

# timemory using headers
set(timemory_FIND_COMPONENTS_INTERFACE timemory-cxx-header-config)
find_package(timemory REQUIRED COMPONENTS ${TIMEMORY_COMMON})

# timemory using library interface in C++
set(timemory_FIND_COMPONENTS_INTERFACE timemory-cxx-library-config)
find_package(timemory REQUIRED COMPONENTS ${TIMEMORY_COMMON} cxx)

define_submodule(
    NAME                timemory_cxx_templates
    LANGUAGE            CXX
    HEADER_FILE         timemory_cxx_templates.hpp
    INTERFACE_LIBRARY   timemory-cxx-header-config
)

define_submodule(
    NAME                timemory_cxx_library
    LANGUAGE            CXX
    HEADER_FILE         timemory_cxx_library.hpp
    INTERFACE_LIBRARY   timemory-cxx-library-config
)

if(TEST_C_TIMEMORY)
    # timemory using library interface in C
    set(timemory_FIND_COMPONENTS_INTERFACE timemory-c-library-config)
    find_package(timemory REQUIRED COMPONENTS ${TIMEMORY_COMMON} c)

    define_submodule(
        NAME                timemory_c_library
        LANGUAGE            C
        HEADER_FILE         timemory_c_library.h
        INTERFACE_LIBRARY   timemory-c-library-config
    )
endif()
```