sge_utils_add_program(
	update_cmake
	"update_cmake.cpp"
	"${Boost_FILESYSTEM_LIBRARY};${Boost_SYSTEM_LIBRARY};${Boost_REGEX_LIBRARY}"
)

target_include_directories(
	update_cmake
	PRIVATE
	"${fcppt_INCLUDE_DIRS}"
)
