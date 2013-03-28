cmake_minimum_required(
	VERSION
	2.8.3
)

project(
	sgeutils
)

include(
	FcpptCMakeUtils
)

# Find Boost
find_package(
	Boost 1.48.0
	REQUIRED
	filesystem
	program_options
	regex
)

include_directories(
	SYSTEM ${Boost_INCLUDE_DIRS}
)

link_directories(
	${Boost_LIBRARY_DIRS}
)

# Find fcppt
find_package(
	fcppt REQUIRED
)

add_definitions(
	${fcppt_DEFINITIONS}
)

include_directories(
	${fcppt_INCLUDE_DIRS}
)

option(
	ENABLE_SGE
	"Use SGE and build utils that require it"
	ON
)

if(
	ENABLE_SGE
)
	find_package(
		sge
		REQUIRED
		COMPONENTS
		parse
	)
endif()

add_subdirectory(
	check_includes
)

if(
	ENABLE_SGE
)
	add_subdirectory(
		compile_headers
	)
endif()

add_subdirectory(
	convenience_headers
)

add_subdirectory(
	file_switch
)

add_subdirectory(
	generate_includeguard
)

add_subdirectory(
	prettify_includes
)

add_subdirectory(
	remove_exec_bit
)

add_subdirectory(
	replace_whitespaces
)

if(
	ENABLE_SGE
)
	add_subdirectory(
		single_compile
	)
endif()

if(
	ENABLE_SGE
)
	add_subdirectory(
		update_license
	)
endif()

add_subdirectory(
	update_cmake
)
