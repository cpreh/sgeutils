sge_utils_add_program(
	single_compile
	"single_compile.cpp"
	"${fcppt_core_TARGET};${sgecore_TARGET};${sgeparsejson_TARGET};${Boost_FILESYSTEM_LIBRARY};${Boost_PROGRAM_OPTIONS_LIBRARY};${Boost_REGEX_LIBRARY}"
)
