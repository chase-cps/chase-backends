###############################################################################
# CMake file for the CHASE Library standard backends.
# @author Michele Lora
###############################################################################

cmake_minimum_required (VERSION 3.10)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_C_FLAGS_DEBUG "-O0 -Wall -Wextra -pedantic -ggdb -fPIC")
set(CMAKE_C_FLAGS_RELEASE "-O3 -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wextra -pedantic -ggdb -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3  -fPIC")

set(CMAKE_INSTALL_PREFIX ../../install_dir CACHE PATH "Install top-level directory" FORCE)



find_package(PythonLibs REQUIRED)
find_package(pybind11 REQUIRED)

project(CHASE_BACKENDS VERSION 1.0)

set(CHASEPATH ../../install_dir CACHE PATH "The path of the Chase installation")

set( BACKENDS_SRC ${PROJECT_SOURCE_DIR}/src)


set(chase_library_backends
        ${BACKENDS_SRC}/GR1CPrinter.cc
        ${BACKENDS_SRC}/GR1CPrinter_visit.cc
        ${BACKENDS_SRC}/SlugsPrinter.cc
        ${BACKENDS_SRC}/SlugsPrinter_visit.cc
        ${BACKENDS_SRC}/NuSMVPrinter.cc
        ${BACKENDS_SRC}/NuSMVPrinter_visit.cc
        ${BACKENDS_SRC}/PySTLPrinter.cc
)

include_directories(
    ${CHASEPATH}/chase/include
    ${CHASEPATH}/chase/include/utilities
    ${CHASEPATH}/chase/include/representation
    ${PROJECT_SOURCE_DIR}/include
    )

add_library(chase-backends ${chase_library_backends})

include(GNUInstallDirs)
set(BACKENDSLIB_INSTALL_DIR chase/lib  CACHE STRING ¨¨)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION chase/include/backends
    )

install(
    TARGETS chase-backends
    EXPORT chaseTargets
    DESTINATION chase/lib
    )

pybind11_add_module(
    pychase_backends MODULE
    src/bindings.cc
    ${chase_library_backends}
    )

target_link_libraries(pychase_backends PRIVATE chase)

target_link_directories(
    pychase_backends
    PRIVATE ${CHASEPATH}/chase/lib
    )

install(TARGETS pychase_backends
    EXPORT pychase_backendsTargets
    DESTINATION chase/lib)

