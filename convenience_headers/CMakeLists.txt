ADD_EXECUTABLE(
	convenience_headers
	convenience_headers.cpp
)

TARGET_LINK_LIBRARIES(
	convenience_headers	
	${Fcppt_LIBRARIES}
)

INSTALL(
	TARGETS convenience_headers
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/
)
