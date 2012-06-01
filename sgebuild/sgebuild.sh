#!/bin/bash

CURRENT_DIR="$(dirname $(readlink -f $0))"

source "${CURRENT_DIR}/common.sh"


# Read the main config

SGEBUILD_CONFIG="${SGEBUILD_CONFIG_PATH}/config.sh"

check_file "${SGEBUILD_CONFIG}"

source "${SGEBUILD_CONFIG}"

check_variable "SGEBUILD_CMAKE_GENERATOR" "${SGEBUILD_CONFIG}"

check_variable "SGEBUILD_BUILD_COMMAND" "${SGEBUILD_CONFIG}"


# Set build variables, they can be used in the sourced files below

SGEBUILD_TARGET_TYPE="$1"

[[ -z "${SGEBUILD_TARGET_TYPE}" ]] && die "You have to provide a build type as first parameter!"

SGEBUILD_BUILD_DIR="sgebuild_${SGEBUILD_TARGET_TYPE}"


# Read the project config

SGEBUILD_PROJECT_CONFIG="${SGEBUILD_PROJECT_CONFIG_PATH}/config.sh"

check_file "${SGEBUILD_PROJECT_CONFIG}"

source "${SGEBUILD_PROJECT_CONFIG}"

check_variable "SGEBUILD_PROJECT_CMAKE_SETTINGS" "${SGEBUILD_PROJECT_CONFIG}"


# Read the target config

SGEBUILD_TARGET_CONFIG="${SGEBUILD_CONFIG_PATH}/${SGEBUILD_TARGET_TYPE}.sh"

check_file "${SGEBUILD_TARGET_CONFIG}"

source "${SGEBUILD_TARGET_CONFIG}"

check_variable "SGEBUILD_C_COMPILER" "${SGEBUILD_TARGET_CONFIG}"

check_variable "SGEBUILD_CXX_COMPILER" "${SGEBUILD_TARGET_CONFIG}"


# Read possible additional config for this target and project

SGEBUILD_PROJECT_TARGET_CONFIG="${SGEBUILD_PROJECT_CONFIG_PATH}/${SGEBUILD_TARGET_TYPE}.sh"

[[ -e "${SGEBUILD_PROJECT_TARGET_CONFIG}" ]] && source "${SGEBUILD_PROJECT_TARGET_CONFIG}"


# Start the build process

mkdir -p "${SGEBUILD_BUILD_DIR}" || die "mkdir ${SGEBUILD_BUILD_DIR} failed"

pushd "${SGEBUILD_BUILD_DIR}" >> /dev/null

export CFLAGS="${CFLAGS} ${SGEBUILD_C_FLAGS}"

export CXXFLAGS="${CXXFLAGS} ${SGEBUILD_CXX_FLAGS}"

cmake \
	-G "${SGEBUILD_CMAKE_GENERATOR}" \
	-D CMAKE_C_COMPILER="${SGEBUILD_C_COMPILER}" \
	-D CMAKE_CXX_COMPILER="${SGEBUILD_CXX_COMPILER}" \
	"${SGEBUILD_TARGET_CMAKE_SETTINGS[@]}" \
	"${SGEBUILD_PROJECT_CMAKE_SETTINGS[@]}" \
	"${SGEBUILD_PROJECT_TARGET_CMAKE_SETTINGS[@]}" \
	.. || die "cmake failed"

"${SGEBUILD_BUILD_COMMAND}" ${@:2} || die "Building failed"

popd > /dev/null
