sge_utils_add_program(
	file_switch
	SOURCES
		file_switch.cpp
	LINK_LIBS
		${FCPPT_UTILS_FILESYSTEM_LIBRARIES}
		${fcppt_core_TARGET}
)
