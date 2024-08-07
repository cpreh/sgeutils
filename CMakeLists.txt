cmake_minimum_required(VERSION 3.27.0)

project(sgeutils)

include(GNUInstallDirs)

option(BUILD_APPS "Build applications which requires fcppt and Boost" ON)

if(BUILD_APPS)
  # Find Boost
  find_package(Boost 1.70.0 CONFIG REQUIRED)

  # Find fcppt
  find_package(fcppt REQUIRED)

  set(FCPPT_UTILS_CLANG_TIDY_WHITELIST "fcppt" "awl" "mizuiro" "sge")

  include(FcpptCMakeUtils)
endif()

option(ENABLE_SGE "Use SGE and build utils that require it" ON)

if(ENABLE_SGE AND BUILD_APPS)
  find_package(Boost 1.70.0 CONFIG REQUIRED COMPONENTS locale)

  find_package(sge REQUIRED COMPONENTS charconv parse parsejson)
endif()

include(CMakeParseArguments)

function(sge_utils_add_program PROGRAM_NAME)
  if(NOT BUILD_APPS)
    return()
  endif()

  set(MULTI_ARGS COMPILE_FLAGS COMPILE_DEFINITIONS COMPILE_OPTIONS LINK_LIBS
                 SOURCES)

  cmake_parse_arguments("" "" "" "${MULTI_ARGS}" ${ARGN})

  if(NOT "${_UNPARSED_ARGUMENTS}" STREQUAL "")
    message(FATAL_ERROR "Invalid parameters ${_UNPARSED_ARGUMENTS}")
  endif()

  add_executable(${PROGRAM_NAME} ${_SOURCES})

  fcppt_utils_set_target_compiler_flags(${PROGRAM_NAME} ADDITIONAL_FLAGS
                                        ${_COMPILE_FLAGS})

  target_link_libraries(${PROGRAM_NAME} PRIVATE ${_LINK_LIBS})

  target_compile_definitions(${PROGRAM_NAME} PRIVATE ${_COMPILE_DEFINITIONS})

  target_compile_options(${PROGRAM_NAME} PRIVATE ${_COMPILE_OPTIONS})

  install(TARGETS ${PROGRAM_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction()

add_subdirectory(check_includes)

add_subdirectory(file_switch)

add_subdirectory(generate_includeguard)

add_subdirectory(prettify_includes)

add_subdirectory(sgebuild)

add_subdirectory(update_headers)

add_subdirectory(remove_exec_bit)

add_subdirectory(replace_whitespaces)

if(ENABLE_SGE)
  add_subdirectory(update_license)
endif()

add_subdirectory(update_cmake)
