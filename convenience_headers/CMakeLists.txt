ADD_EXECUTABLE(
	convenience_headers
	convenience_headers.cpp
)

TARGET_LINK_LIBRARIES(
	convenience_headers
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
)

INSTALL(
	TARGETS convenience_headers
	DESTINATION ${INSTALL_BINARY_DIR}
)
