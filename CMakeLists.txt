cmake_minimum_required(VERSION 3.24)
project(
  lemon
  VERSION 1.2.0
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

# We expose publicly the include files in the source directory
# at build time and the include files in the installation directory
# at installation time.
# We do this to ensure that include directories can be found correctly
# when lemon is used in a CMake project via FetchContent_Declare / FetchContent_MakeAvailable
# (which does not perform the installation step) and when lemon is installed somewhere.
target_include_directories(lemon
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}>
                           )

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
    target_link_libraries(${_progs} PRIVATE lemon)
    target_include_directories(${_progs} 
      PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
endforeach()

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/lemonConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/lemonConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/lemonConfig.cmake" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/lemon.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/lemon.pc" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/lemonConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/lemonConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(FILES "${PROJECT_BINARY_DIR}/lemon.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(
  TARGETS lemon
  EXPORT lemon_targets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(
  EXPORT lemon_targets
  FILE lemonTargets.cmake
  NAMESPACE lemon::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

list(
  APPEND
  lemon_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/lemon_binheader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/lemon.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/lemon_writer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/lemon_defines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/lemon_header.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/lemon_reader.h
)
foreach(_inc ${lemon_headers})
  install(
    FILES "${_inc}"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}"
  )
endforeach()

foreach(_app lemon_contents lemon_benchmark)
  install(TARGETS "${_app}"
          RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endforeach()

