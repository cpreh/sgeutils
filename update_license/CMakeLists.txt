FIND_PACKAGE(
	SGE REQUIRED
	COMPONENTS parse
)

INCLUDE_DIRECTORIES(
	${SGE_INCLUDE_DIRS}
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
	${FCPPT_LIBRARIES}
)

ADD_EXECUTABLE(
	multi_update_license
	multi_update_license.cpp
)

TARGET_LINK_LIBRARIES(
	multi_update_license
	${FCPPT_LIBRARIES}
	${SGE_LIBRARIES}
	${Boost_LIBRARIES}
)
