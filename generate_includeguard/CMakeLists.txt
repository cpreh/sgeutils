ADD_EXECUTABLE(
	generate_includeguard
	generate_includeguard.cpp
)

TARGET_LINK_LIBRARIES(
	generate_includeguard
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
)

INSTALL(
	TARGETS generate_includeguard
	DESTINATION ${INSTALL_BINARY_DIR}
)
