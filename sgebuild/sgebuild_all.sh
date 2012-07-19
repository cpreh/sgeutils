#!/bin/bash

CURRENT_DIR="$(dirname $(readlink -f $0))"

source "${CURRENT_DIR}/common.sh"


# Read the main config

SGEBUILD_ALL_CONFIG="${SGEBUILD_CONFIG_PATH}/all_config.sh"

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
