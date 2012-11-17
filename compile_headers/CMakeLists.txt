FIND_PACKAGE(
	sge REQUIRED
)

INCLUDE_DIRECTORIES(
	${sge_INCLUDE_DIRS}
)

ADD_EXECUTABLE(
	compile_headers
	compile_headers.cpp
)

TARGET_LINK_LIBRARIES(
	compile_headers
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
	${sgecore_TARGET}
	${sgeparse_TARGET}
)

INSTALL(
	TARGETS compile_headers
	DESTINATION ${INSTALL_BINARY_DIR}
)
