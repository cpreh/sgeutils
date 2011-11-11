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
	Fcppt REQUIRED
)

FIND_PACKAGE(Fcppt REQUIRED)

ADD_DEFINITIONS(${Fcppt_DEFINITIONS})

INCLUDE_DIRECTORIES(${Fcppt_INCLUDE_DIRS})

IF(
	UNIX
)
	ADD_SUBDIRECTORY(
		build_headers
	)
ENDIF()

ADD_SUBDIRECTORY(
	check_includes
)

ADD_SUBDIRECTORY(
	convenience_headers
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
