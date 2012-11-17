FIND_PACKAGE(
	Boost REQUIRED
	filesystem
)

ADD_EXECUTABLE(
	file_switch
	file_switch.cpp
)

TARGET_LINK_LIBRARIES(
	file_switch
	${fcppt_core_TARGET}
	${Boost_FILESYSTEM_LIBRARIES}
)

INSTALL(
	TARGETS file_switch
	DESTINATION ${INSTALL_BINARY_DIR}
)
