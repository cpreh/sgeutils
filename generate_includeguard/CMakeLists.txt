sge_utils_add_program(
  generate_includeguard
  SOURCES
  generate_includeguard.cpp
  COMPILE_OPTIONS
  ${FCPPT_UTILS_MAIN_COMPILE_OPTIONS}
  LINK_LIBS
  ${FCPPT_UTILS_MAIN_LINK_LIBS}
  ${fcppt_core_TARGET}
  ${fcppt_filesystem_TARGET}
  ${fcppt_options_TARGET})
