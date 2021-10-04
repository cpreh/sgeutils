if(WIN32)
	set(ASIO_LIBS ws2_32)
elseif(UNIX)
	set(ASIO_LIBS pthread)
	set(ASIO_COMPILE_OPTIONS -pthread)
endif()

sge_utils_add_program(
	compile_headers
	SOURCES
		compile_headers.cpp
	COMPILE_DEFINITIONS
		BOOST_ASIO_HAS_STD_INVOKE_RESULT
	COMPILE_OPTIONS
		${FCPPT_UTILS_MAIN_COMPILE_OPTIONS}
		${ASIO_COMPILE_OPTIONS}
	LINK_LIBS
		${FCPPT_UTILS_MAIN_LINK_LIBS}
		${fcppt_core_TARGET}
		${fcppt_filesystem_TARGET}
		${fcppt_options_TARGET}
		${sgecharconv_TARGET}
		${sgecore_TARGET}
		${sgeparsejson_TARGET}
		${ASIO_LIBS}
)
