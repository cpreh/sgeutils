sge_utils_add_program(
	update_license
	"update_license.cpp"
	""
)

target_include_directories(
	update_license
	PRIVATE
	"${fcppt_INCLUDE_DIRS}"
)

sge_utils_add_program(
	multi_update_license
	"multi_update_license.cpp"
	"${fcppt_core_TARGET};${fcppt_filesystem_TARGET};${sgecore_TARGET};${sgeparse_TARGET};${sgeparsejson_TARGET};${Boost_REGEX_LIBRARY}"
)
