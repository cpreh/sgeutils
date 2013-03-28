add_executable(
	check_includes
	check_includes.cpp
)

target_link_libraries(
	check_includes
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
)

install(
	TARGETS check_includes
	DESTINATION ${INSTALL_BINARY_DIR}
)
