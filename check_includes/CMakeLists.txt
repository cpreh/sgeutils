sge_utils_add_program(
	check_includes
	SOURCES
		check_includes.cpp
	LINK_LIBS
		${fcppt_core_TARGET}
		${fcppt_filesystem_TARGET}
)
