add_executable(
	generate_includeguard
	generate_includeguard.cpp
)

target_link_libraries(
	generate_includeguard
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
)

install(
	TARGETS generate_includeguard
	DESTINATION ${INSTALL_BINARY_DIR}
)
