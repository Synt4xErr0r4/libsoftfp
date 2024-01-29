cmake_minimum_required(VERSION 3.24)

project(softfp
    VERSION 0.1.0
    DESCRIPTION "A software floating-point library"
    HOMEPAGE_URL "https://github.com/Synt4xErr0r4/libsoftfp"
    LANGUAGES C)

file(GLOB_RECURSE sources src/*.c)

include(GNUInstallDirs)

add_library(softfp SHARED ${sources})

target_compile_options(softfp PRIVATE -Wno-builtin-declaration-mismatch)

set_target_properties(softfp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/softfp.h)

target_include_directories(softfp PRIVATE include)

configure_file(libsoftfp.pc.in libsoftfp.pc @ONLY)

install(TARGETS softfp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/libsoftfp.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
