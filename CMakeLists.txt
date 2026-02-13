cmake_minimum_required(VERSION 3.24)
project(
  lemon
  VERSION 0.99
  HOMEPAGE_URL "https://github.com/etmc/lemon"
  DESCRIPTION "Lemon is an MPI parallel I/O library that is intended to allow for efficient parallel I/O of both binary and metadata on massively parallel architectures. Data is stored in the SciDAC Lattice QCD Interchange Message Encapsulation format, that allows for storing large blocks of binary data and corresponding metadata in the same file."
  LANGUAGES C)

option(CMAKE_POSITION_INDEPENDENT_CODE "Enable position independent code" ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Boiler Plate for installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

# Boiler Plate for Testing
include(CTest)

find_package(MPI REQUIRED)

list(
  APPEND
  LEMON_SRCS
  src/header_construct.c
  src/reader_closeRecord.c
  src/reader_nextRecord.c
  src/writer_construct.c
  src/header_destroy.c
  src/reader_construct.c
  src/reader_padBytes.c
  src/writer_destroy.c
  src/reader_destroy.c
  src/reader_readData.c
  src/writer_latticeParallel.c
  src/reader_EOM.c
  src/reader_seek.c
  src/writer_recordData.c
  src/reader_getPointer.c
  src/reader_setPointer.c
  src/writer_recordHeader.c
  src/reader_latticeParallel.c
  src/reader_setState.c
  src/writer_seek.c
  src/reader_MBFlag.c
  src/reader_type.c
  src/writer_setState.c
  src/reader_bytes.c
  src/reader_MEFlag.c
  src/writer_closeRecord.c
  src/reader_finishReading.c
  src/writer_finishWriting.c
  src/reader_readDataNonBlocking.c
  src/reader_latticeParallelNonBlocking.c
  src/writer_latticeParallelNonBlocking.c
  src/writer_latticeParallelMapped.c
  src/reader_latticeParallelMapped.c
  src/writer_latticeParallelNonBlockingMapped.c
  src/reader_latticeParallelNonBlockingMapped.c
  src/writer_recordDataNonBlocking.c)

add_library(lemon ${LEMON_SRCS})
add_library(lemon::lemon ALIAS lemon)
target_link_libraries(lemon PUBLIC MPI::MPI_C m)
target_include_directories(lemon
                           PUBLIC $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

include_directories(
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

add_executable(lemon_contents check/lemon_contents.c)
add_executable(lemon_benchmark check/lemon_benchmark.c check/md5.c
                               check/humanForm.c)
add_executable(canonical check/canonical.c)
add_executable(parallel check/parallel.c)
add_executable(xlf check/xlf.c)
add_executable(parallel_non_blocking check/parallel_non_blocking.c)
add_executable(xlf_non_blocking check/xlf_non_blocking.c)

foreach(
  _progs
  lemon_contents
  lemon_benchmark
  canonical
  parallel
  xlf
  parallel_non_blocking
  xlf_non_blocking)
  target_link_libraries(${_progs} PUBLIC lemon)
endforeach()

list(
  APPEND
  lemon_headers
  ${PROJECT_SOURCE_DIR}/include/lemon_binheader.h
  ${PROJECT_SOURCE_DIR}/include/lemon.h
  ${PROJECT_SOURCE_DIR}/include/lemon_writer.h
  ${PROJECT_SOURCE_DIR}/include/lemon_defines.h
  ${PROJECT_SOURCE_DIR}/include/lemon_header.h
  ${PROJECT_SOURCE_DIR}/include/lemon_reader.h)

include(GNUInstallDirs)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/lemonConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion)

configure_file("${PROJECT_SOURCE_DIR}/cmake/lemonConfig.cmake.in"
               "${PROJECT_BINARY_DIR}/lemonConfig.cmake" @ONLY)

configure_file("${PROJECT_SOURCE_DIR}/cmake/lemon.pc.in"
               "${PROJECT_BINARY_DIR}/lemon.pc" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/lemonConfig.cmake"
              "${PROJECT_BINARY_DIR}/lemonConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(FILES "${PROJECT_BINARY_DIR}/lemon.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(
  TARGETS lemon
  EXPORT lemon_targets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(
  EXPORT lemon_targets
  FILE lemonTargets.cmake
  NAMESPACE lemon::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

foreach(_app lemon_contents lemon_benchmark)
  install(FILES "${CMAKE_BINARY_DIR}/${_app}"
          DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

foreach(_inc ${lemon_headers})
  install(FILES "${_inc}"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
endforeach()
