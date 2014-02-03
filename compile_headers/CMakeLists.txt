include_directories(
	${sge_INCLUDE_DIRS}
)

add_executable(
	compile_headers
	compile_headers.cpp
)

target_link_libraries(
	compile_headers
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
	${sgecore_TARGET}
	${sgeparsejson_TARGET}
)

install(
	TARGETS compile_headers
	DESTINATION ${INSTALL_BINARY_DIR}
)
