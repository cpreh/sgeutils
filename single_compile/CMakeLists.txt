include_directories(
	${sge_INCLUDE_DIRS}
)

add_executable(
	single_compile
	single_compile.cpp
)

target_link_libraries(
	single_compile
	${fcppt_core_TARGET}
	${sgecore_TARGET}
	${sgeparsejson_TARGET}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_REGEX_LIBRARY}
)

install(
	TARGETS single_compile
	DESTINATION ${INSTALL_BINARY_DIR}
)
