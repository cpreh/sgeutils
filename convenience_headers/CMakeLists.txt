sge_utils_add_program(
	convenience_headers
	SOURCES
		convenience_headers.cpp
	LINK_LIBS
		${fcppt_core_TARGET}
		${fcppt_filesystem_TARGET}
)
