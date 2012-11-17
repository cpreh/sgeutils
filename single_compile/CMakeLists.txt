FIND_PACKAGE(
	sge REQUIRED
)

INCLUDE_DIRECTORIES(
	${sge_INCLUDE_DIRS}
)

FIND_PACKAGE(
	Boost 1.40.0 REQUIRED
	COMPONENTS program_options regex
)

ADD_EXECUTABLE(
	single_compile
	single_compile.cpp
)

TARGET_LINK_LIBRARIES(
	single_compile
	${fcppt_core_TARGET}
	${sgecore_TARGET}
	${sgeparse_TARGET}
	${Boost_FILESYSTEM_LIBRARIES}
	${Boost_PROGRAM_OPTIONS_LIBRARIES}
	${Boost_REGEX_LIBRARIES}
)

INSTALL(
	TARGETS single_compile
	DESTINATION ${INSTALL_BINARY_DIR}
)
