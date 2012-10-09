#!/bin/bash

CURRENT_DIR="$(dirname "$(readlink -f "$0")")"

source "${CURRENT_DIR}/common.sh"


# Setup main config

SGEBUILD_CONFIG="${SGEBUILD_CONFIG_PATH}/config.sh"

SGEBUILD_TARGET_TYPE="$1"

[[ -z "${SGEBUILD_TARGET_TYPE}" ]] && die "You have to provide a build type as first parameter!"


# Display help message

if [[ "${SGEBUILD_TARGET_TYPE}" == "--help" ]] ; then

echo "
Usage: $0 <target> [parameters]

sgebuild tries to unify the build process for different compilers, cmake
generators, build options, and so on. They are collectively called targets
here.

Parameters:
  - target: The target to build
  - parameters: Additional parameters that are passed to the build command

To assist in the building process, sgebuild tries to source several config
files. Assuming your target is called MYTARGET, the following files are
sourced in the following order:
  - The sgebuild config file: ${SGEBUILD_CONFIG}
  - The target config file: ${SGEBUILD_CONFIG_PATH}/MYTARGET.sh
  - The project config file: ${SGEBUILD_PROJECT_CONFIG_PATH}/config.sh
  - The project target config file: ${SGEBUILD_PROJECT_CONFIG_PATH}/MYTARGET.sh

Notice, that the project (target) config files are searched for relative to the
current working directory.

All of the above files can be used to set anything that might be needed for
calling cmake. What comes last overwrites what comes first.

The following variables are read from the config files and passed to cmake:

sgebuild config (mandatory):
  SGEBUILD_CMAKE_GENERATOR:
    The default cmake generator to use (see cmake -G)

  SGEBUILD_BUILD_COMMAND:
    The default build command to use (for example make). Everything in
    [parameters] is passed to this command.

target config (mandatory):
  SGEBUILD_TARGET_CMAKE_SETTINGS:
    An array of cmake settings to apply on a target wide basis, for example:

    SGEBUILD_TARGET_CMAKE_SETTINGS=(
      \"-DCMAKE_C_COMPILER=/usr/bin/gcc\"
      \"-DCMAKE_CXX_COMPILER=/usr/bin/g++\"
    )

project config (mandatory):
  SGEBUILD_PROJECT_CMAKE_SETTINGS:
    An array of cmake settings to apply on a project wide basis, for example:

    SGEBUILD_PROJECT_CMAKE_SETTINGS=(
      \"-DENABLE_FOO=ON\"
      \"-DFoo_INCLUDE_DIR=\"\${HOME}/foo/include\"
    )

project target config (optional):
  SGEBUILD_PROJECT_TARGET_CMAKE_SETTINGS:
    An array of cmake settings to apply to a project and target only, for
    example:

    SGEBUILD_PROJECT_TARGET_CMAKE_SETTINGS=(
      \"-DENABLE_NARROW_STRING=OFF\"
    )

The following variables are defined by sgebuild to help you define the above
config files:

SGEBUILD_TARGET_TYPE:
  The same as <target>.

SGEBUILD_BUILD_DIR_PREFIX:
  The prefix sgebuild uses to name build dirs.

SGEBUILD_BUILD_DIR:
  The build directory for the current target, which is
  \"\${SGEBUILD_BUILD_DIR_PREFIX}\${SGEBUILD_TARGET_TYPE}\"

  For example, to refer to another project that is also built using sgebuild,
  you could use:

  SGEBUILD_PROJECT_CMAKE_SETTINGS=(
    \"-DFoo_LIBRARY_DIR=\"\${HOME}/foo/\${SGEBUILD_BUILD_DIR}/lib\"
  )
"

exit

fi

# Read main config

check_file "${SGEBUILD_CONFIG}"

source "${SGEBUILD_CONFIG}"

check_variable "SGEBUILD_CMAKE_GENERATOR" "${SGEBUILD_CONFIG}"

check_variable "SGEBUILD_BUILD_COMMAND" "${SGEBUILD_CONFIG}"


# Set build variables, which can be used in the sourced files below

SGEBUILD_BUILD_DIR_PREFIX="sgebuild_"

SGEBUILD_BUILD_DIR="${SGEBUILD_BUILD_DIR_PREFIX}${SGEBUILD_TARGET_TYPE}"

SGEBUILD_PROJECT_CONFIG="${SGEBUILD_PROJECT_CONFIG_PATH}/config.sh"

SGEBUILD_TARGET_CONFIG="${SGEBUILD_CONFIG_PATH}/${SGEBUILD_TARGET_TYPE}.sh"

SGEBUILD_PROJECT_TARGET_CONFIG="${SGEBUILD_PROJECT_CONFIG_PATH}/${SGEBUILD_TARGET_TYPE}.sh"


# Read the target config

check_file "${SGEBUILD_TARGET_CONFIG}"

source "${SGEBUILD_TARGET_CONFIG}"

# The generator can be overwritten in target files

[[ -n "${SGEBUILD_TARGET_CMAKE_GENERATOR}" ]] && SGEBUILD_CMAKE_GENERATOR="${SGEBUILD_TARGET_CMAKE_GENERATOR}"


# Read the project config

check_file "${SGEBUILD_PROJECT_CONFIG}"

source "${SGEBUILD_PROJECT_CONFIG}"

check_variable "SGEBUILD_PROJECT_CMAKE_SETTINGS" "${SGEBUILD_PROJECT_CONFIG}"


# Read possible additional config for this target and project

[[ -e "${SGEBUILD_PROJECT_TARGET_CONFIG}" ]] && source "${SGEBUILD_PROJECT_TARGET_CONFIG}"


# Start the build process

mkdir -p "${SGEBUILD_BUILD_DIR}" || die "mkdir ${SGEBUILD_BUILD_DIR} failed"

pushd "${SGEBUILD_BUILD_DIR}" >> /dev/null

export CFLAGS="${CFLAGS} ${SGEBUILD_C_FLAGS}"

export CXXFLAGS="${CXXFLAGS} ${SGEBUILD_CXX_FLAGS}"

cmake \
	-G "${SGEBUILD_CMAKE_GENERATOR}" \
	"${SGEBUILD_TARGET_CMAKE_SETTINGS[@]}" \
	"${SGEBUILD_PROJECT_CMAKE_SETTINGS[@]}" \
	"${SGEBUILD_PROJECT_TARGET_CMAKE_SETTINGS[@]}" \
	.. || die "cmake failed"

"${SGEBUILD_BUILD_COMMAND}" "${@:2}" || die "Building failed"

popd > /dev/null
