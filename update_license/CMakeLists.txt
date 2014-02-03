include_directories(
	${sge_INCLUDE_DIRS}
)

add_executable(
	update_license
	update_license.cpp
)

install(
	TARGETS update_license
	DESTINATION ${INSTALL_BINARY_DIR}
)

add_executable(
	multi_update_license
	multi_update_license.cpp
)

target_link_libraries(
	multi_update_license
	${fcppt_core_TARGET}
	${fcppt_filesystem_TARGET}
	${sgecore_TARGET}
	${sgeparse_TARGET}
	${sgeparsejson_TARGET}
	${Boost_REGEX_LIBRARY}
)

install(
	TARGETS multi_update_license
	DESTINATION ${INSTALL_BINARY_DIR}
)
