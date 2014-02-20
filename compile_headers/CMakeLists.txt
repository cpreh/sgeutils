sge_utils_add_program(
	compile_headers
	"compile_headers.cpp"
	"${fcppt_core_TARGET};${fcppt_filesystem_TARGET};${sgecore_TARGET};${sgeparsejson_TARGET}"
)
