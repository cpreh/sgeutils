add_executable(
	file_switch
	file_switch.cpp
)

target_link_libraries(
	file_switch
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${fcppt_core_TARGET}
)

install(
	TARGETS file_switch
	DESTINATION ${INSTALL_BINARY_DIR}
)
