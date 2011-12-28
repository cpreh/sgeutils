ADD_EXECUTABLE(
	generate_includeguard
	generate_includeguard.cpp
)

TARGET_LINK_LIBRARIES(
	generate_includeguard
	${Fcppt_core_LIBRARIES}
	${Fcppt_filesystem_LIBRARIES}
)

INSTALL(
	TARGETS generate_includeguard
	DESTINATION ${INSTALL_BINARY_DIR}
)
