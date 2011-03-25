ADD_EXECUTABLE(
	update_cmake
	update_cmake.cpp
)

TARGET_LINK_LIBRARIES(
	update_cmake
	${FCPPT_LIBRARIES}
)
