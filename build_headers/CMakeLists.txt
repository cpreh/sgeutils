ADD_EXECUTABLE(
	build_headers
	build_headers.cpp
)

TARGET_LINK_LIBRARIES(
	build_headers
	${Fcppt_core_LIBRARIES}
	${Fcppt_filesystem_LIBRARIES}
)

INSTALL(
	TARGETS build_headers
	DESTINATION ${INSTALL_BINARY_DIR}
)
