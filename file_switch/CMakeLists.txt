add_executable(
	file_switch
	file_switch.cpp
)

target_link_libraries(
	file_switch
	${fcppt_core_TARGET}
	${Boost_FILESYSTEM_LIBRARY}
)

install(
	TARGETS file_switch
	DESTINATION ${INSTALL_BINARY_DIR}
)
