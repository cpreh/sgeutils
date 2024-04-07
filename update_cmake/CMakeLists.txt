sge_utils_add_program(
	update_cmake
	SOURCES
		update_cmake.cpp
	LINK_LIBS
		fcppt_core_interface
)

install(
	PROGRAMS
	update_cmake.sh
	DESTINATION
	${CMAKE_INSTALL_BINDIR}
)
