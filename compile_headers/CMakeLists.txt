FIND_PACKAGE(
	SGE REQUIRED
	COMPONENTS core parse
)

INCLUDE_DIRECTORIES(
	${SGE_INCLUDE_DIRS}
)

ADD_EXECUTABLE(
	compile_headers
	compile_headers.cpp
)

TARGET_LINK_LIBRARIES(
	compile_headers
	${Fcppt_core_LIBRARIES}
	${Fcppt_filesystem_LIBRARIES}
	${SGE_LIBRARIES}
)

INSTALL(
	TARGETS compile_headers
	DESTINATION ${INSTALL_BINARY_DIR}
)
