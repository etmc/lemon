cmake_minimum_required(VERSION 3.24)
project(
  EtmcLemon
  VERSION 0.99
  LANGUAGES C)

# Boiler Plate for installation
include(GNUInstallDirs)
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CMakePackageConfigHelpers)

# Boiler Plate for Testing
include(CTest)

find_package(MPI REQUIRED)

check_symbol_exists(gettimeofday "time.h" HAVE_GET_TIMEOFDAY)

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
add_library(etmc_lemon::lemon ALIAS lemon)
target_link_libraries(lemon MPI::MPI_C m)
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
  lemon_binheader.h
  lemon.h
  lemon_writer.h
  lemon_defines.h
  lemon_header.h
  lemon_reader.h)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/EtmcLemonConfigVersion.cmake"
  VERSION "${CP2K_VERSION}"
  COMPATIBILITY SameMajorVersion)

configure_file(cmake/config.h.in config.h)

configure_file("${PROJECT_SOURCE_DIR}/cmake/EtmcLemonConfig.cmake.in"
               "${PROJECT_BINARY_DIR}/EtmcLemonConfig.cmake" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/EtmcLemonConfig.cmake"
              "${PROJECT_BINARY_DIR}/EtmcLemonConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/etmc_lemon")

install(FILES "${PROJECT_BINARY_DIR}/etmc-lemon.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(
  TARGETS lemon
  EXPORT lemon_targets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(
  EXPORT lemon_targets
  FILE lemonTargets.cmake
  NAMESPACE etmc_lemon::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(FILES ${PROJECT_SOURCE_DIR}/include/${lemon_headers}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
