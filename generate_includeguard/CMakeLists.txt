sge_utils_add_program(
	generate_includeguard
	SOURCES
		generate_includeguard.cpp
	LINK_LIBS
		${fcppt_core_TARGET}
		${fcppt_filesystem_TARGET}
)
