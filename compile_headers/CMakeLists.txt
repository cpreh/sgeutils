find_package(
	FcpptThreads
	REQUIRED
)

sge_utils_add_program(
	compile_headers
	SOURCES
		compile_headers.cpp
	COMPILE_FLAGS
		${FcpptThreads_DEFINITIONS}
	LINK_LIBS
		${FcpptThreads_LIBRARIES}
		${fcppt_core_TARGET}
		${fcppt_filesystem_TARGET}
		${sgecore_TARGET}
		${sgeparsejson_TARGET}
)
