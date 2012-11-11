#!/bin/bash

CURRENT_DIR="$(dirname "$(readlink -f "$0")")"

source "${CURRENT_DIR}/common.sh"


# Setup main config

SGEBUILD_ALL_CONFIG="${SGEBUILD_CONFIG_PATH}/all_config.sh"

function parameter_error() {
	echo "Either pass --help or nothing"
	exit -1
}

if [[ $# > 1 ]] ; then
	parameter_error
fi

# Display help message

if [[ -n "$1" ]] ; then
	if [[ "$1" != "--help" ]] ; then
		parameter_error
	fi

echo "
Usage: $0

This command builds all sgebuild configs in one go. It relies on sgebuild.sh,
therefore see sgebuild.sh --help, too.

Two additional config files are read to determine which targets to build in the
following order:
  - The sgebuild all config file: ${SGEBUILD_ALL_CONFIG}
  - The project all config file: ${SGEBUILD_PROJECT_CONFIG_PATH}/all_config.sh

Notice, that the project config files are searched for relative to the current
working directory.

The following variables are read from the config files:

sgebuild all config (mandatory):
  SGEBUILD_ALL_TARGETS:
    An array of targets to build for all projects, for example:

    SGEBUILD_ALL_TARGETS=(
      \"clang\"
      \"gcc\"
    )

project all config (optional):
  SGEBUILD_ALL_PROJECT_TARGETS:
    An array of targets to additionally build for this project, for example:

    SGEBUILD_ALL_PROJECT_TARGETS=(
      \"gcc_flto\"
    )
"

exit

fi

# Read the main config

check_file "${SGEBUILD_ALL_CONFIG}"

source "${SGEBUILD_ALL_CONFIG}"

check_variable "SGEBUILD_ALL_TARGETS" "${SGEBUILD_ALL_CONFIG}"


# Read the project config

SGEBUILD_PROJECT_ALL_CONFIG="${SGEBUILD_PROJECT_CONFIG_PATH}/all_config.sh"

[[ -e "${SGEBUILD_PROJECT_ALL_CONFIG}" ]] && source "${SGEBUILD_PROJECT_ALL_CONFIG}"

SGEBUILD_ALL_TARGETS=(
	"${SGEBUILD_ALL_TARGETS[@]}"
	"${SGEBUILD_ALL_PROJECT_TARGETS[@]}"
)

for target in "${SGEBUILD_ALL_TARGETS[@]}" ; do
	ignore=false
	for ignore_target in "${SGEBUILD_ALL_PROJECT_IGNORE_TARGETS[@]}" ; do
		if [[ "${target}" = "${ignore_target}" ]] ; then
			ignore=true
		fi
	done

	if ! ${ignore} ; then
		"${CURRENT_DIR}/sgebuild.sh" "${target}" "$@" || die "Aborted"
	fi
done
