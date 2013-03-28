add_executable(
	convenience_headers
	convenience_headers.cpp
)

target_link_libraries(
	convenience_headers
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
)

install(
	TARGETS convenience_headers
	DESTINATION ${INSTALL_BINARY_DIR}
)
