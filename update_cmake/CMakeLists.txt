add_executable(
	update_cmake
	update_cmake.cpp
)

target_link_libraries(
	update_cmake
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_REGEX_LIBRARY}
)

install(
	TARGETS update_cmake
	DESTINATION ${INSTALL_BINARY_DIR}
)
