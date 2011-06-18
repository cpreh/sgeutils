ADD_EXECUTABLE(
	check_includes
	check_includes.cpp
)

TARGET_LINK_LIBRARIES(
	check_includes
	${Fcppt_LIBRARIES}
)

INSTALL(
	TARGETS check_includes
	DESTINATION ${INSTALL_BINARY_DIR}
)
