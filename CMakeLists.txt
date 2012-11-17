CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)

PROJECT (sgeutils)

INCLUDE(FcpptCMakeUtils)

# Find Boost
FIND_PACKAGE(
	Boost 1.40.0 REQUIRED
)

INCLUDE_DIRECTORIES(
	SYSTEM ${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
	${Boost_LIBRARY_DIRS}
)

# Find fcppt
FIND_PACKAGE(
	fcppt REQUIRED
)

ADD_DEFINITIONS(${fcppt_DEFINITIONS})

INCLUDE_DIRECTORIES(${fcppt_INCLUDE_DIRS})

ADD_SUBDIRECTORY(
	check_includes
)

ADD_SUBDIRECTORY(
	compile_headers
)

ADD_SUBDIRECTORY(
	convenience_headers
)

ADD_SUBDIRECTORY(
	file_switch
)

ADD_SUBDIRECTORY(
	generate_includeguard
)

ADD_SUBDIRECTORY(
	prettify_includes
)

ADD_SUBDIRECTORY(
	remove_exec_bit
)

ADD_SUBDIRECTORY(
	replace_whitespaces
)

OPTION(
	ENABLE_SINGLE_COMPILE
	"Build single_compile (requires sge)"
	ON
)

IF(
	ENABLE_SINGLE_COMPILE
)
	ADD_SUBDIRECTORY(
		single_compile
	)
ENDIF()

OPTION(
	ENABLE_UPDATE_LICENSE
	"Build update_license (requires sge)"
	ON
)

IF(
	ENABLE_UPDATE_LICENSE
)
	ADD_SUBDIRECTORY(
		update_license
	)
ENDIF()

ADD_SUBDIRECTORY(
	update_cmake
)
