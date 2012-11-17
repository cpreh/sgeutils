ADD_EXECUTABLE(
	check_includes
	check_includes.cpp
)

TARGET_LINK_LIBRARIES(
	check_includes
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
)

INSTALL(
	TARGETS check_includes
	DESTINATION ${INSTALL_BINARY_DIR}
)
