FIND_PACKAGE(
	sge REQUIRED
)

INCLUDE_DIRECTORIES(
	${sge_INCLUDE_DIRS}
)

FIND_PACKAGE(
	Boost 1.40.0 REQUIRED
	COMPONENTS regex
)

ADD_EXECUTABLE(
	update_license
	update_license.cpp
)

TARGET_LINK_LIBRARIES(
	update_license
	${Fcppt_core_LIBRARIES}
)

INSTALL(
	TARGETS update_license
	DESTINATION ${INSTALL_BINARY_DIR}
)

ADD_EXECUTABLE(
	multi_update_license
	multi_update_license.cpp
)

TARGET_LINK_LIBRARIES(
	multi_update_license
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
	${sgecore_TARGET}
	${sgeparse_TARGET}
	${Boost_REGEX_LIBRARY}
)

INSTALL(
	TARGETS multi_update_license
	DESTINATION ${INSTALL_BINARY_DIR}
)
