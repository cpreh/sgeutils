find_package(
	sge REQUIRED
)

include_directories(
	${sge_INCLUDE_DIRS}
)

find_package(
	Boost 1.40.0 REQUIRED
	COMPONENTS program_options regex
)

add_executable(
	single_compile
	single_compile.cpp
)

target_link_libraries(
	single_compile
	${fcppt_core_TARGET}
	${sgecore_TARGET}
	${sgeparse_TARGET}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_REGEX_LIBRARY}
)

install(
	TARGETS single_compile
	DESTINATION ${INSTALL_BINARY_DIR}
)
