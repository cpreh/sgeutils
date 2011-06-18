ADD_EXECUTABLE(
	update_cmake
	update_cmake.cpp
)

TARGET_LINK_LIBRARIES(
	update_cmake
	${Fcppt_LIBRARIES}
)

INSTALL(
	TARGETS update_cmake 
	DESTINATION ${INSTALL_BINARY_DIR}
)
