cmake_minimum_required(VERSION 3.13)
project(UselessShell C)

if(MSVC)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
endif()

SET(SOURCE_FILES
    lib/Md5.c

    src/ansi_codes.c
    src/cmd_defs.c
    src/cmd_impls.c
    src/useless_shell.c
    src/us_impl.c
    src/util.c
    src/main.c
)

if(WIN32)
    list(APPEND SOURCE_FILES us.rc)
endif()

add_executable(us ${SOURCE_FILES})

set_property(TARGET us PROPERTY C_STANDARD 11)
set_property(TARGET us PROPERTY C_STANDARD_REQUIRED ON)

target_compile_definitions(us PRIVATE
    $<$<CONFIG:Debug>:
        US_ENABLE_TESTING
    >
)

target_include_directories(us PUBLIC include)
target_include_directories(us PRIVATE lib)
