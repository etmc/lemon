cmake_minimum_required(VERSION 3.24)
project(
  clemon
  VERSION 0.99
  LANGUAGES C)

option(CMAKE_POSITION_INDEPENDENT_CODE "Enable position independent code" ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Boiler Plate for installation
include(GNUInstallDirs)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CMakePackageConfigHelpers)

if(NOT DEFINED CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

# Boiler Plate for Testing
include(CTest)

find_package(MPI REQUIRED)
set(HAVE_LIBMPI ON)

check_symbol_exists(gettimeofday "time.h" HAVE_GET_TIMEOFDAY)
set(HAVE_MALLOC ON)

check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/sys_stats.h HAVE_SYS_STATS_H)
check_include_files(time.h HAVE_TIME_H)
check_include_files(sys/sys_types.h HAVE_SYS_TYPES_H)

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

add_library(lemon SHARED ${LEMON_SRCS})
add_library(lemon::clemon ALIAS lemon)
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
  "${PROJECT_BINARY_DIR}/ClemonConfigVersion.cmake"
  VERSION "${CLEMON_VERSION}"
  COMPATIBILITY SameMajorVersion)

configure_file("${PROJECT_SOURCE_DIR}/cmake/config.h.in"
               "${PROJECT_BINARY_DIR}/config.h" @ONLY)

configure_file("${PROJECT_SOURCE_DIR}/cmake/ClemonConfig.cmake.in"
               "${PROJECT_BINARY_DIR}/ClemonConfig.cmake" @ONLY)

configure_file("${PROJECT_SOURCE_DIR}/cmake/clemon.pc.in"
               "${PROJECT_BINARY_DIR}/clemon.pc" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/ClemonConfig.cmake"
              "${PROJECT_BINARY_DIR}/ClemonConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(FILES "${PROJECT_BINARY_DIR}/clemon.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(
  TARGETS lemon
  EXPORT lemon_targets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(
  EXPORT lemon_targets
  FILE ClemonTargets.cmake
  NAMESPACE clemon::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

foreach(_app lemon_contents lemon_benchmark)
  install(FILES "${CMAKE_BINARY_DIR}/${_app}" DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

foreach(_inc ${lemon_headers})
  install(FILES "${_inc}"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
endforeach()
