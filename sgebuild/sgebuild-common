#!/bin/bash

function die()
{
	echo "$1"

	exit -1
}

function check_file()
{
	[[ -e "$1" ]] || die "$1 not found!"
}

function check_variable()
{
	[[ -z "${!1}" ]] && die "$1 must be defined in $2!"
}

if [[ -z "${XDG_CONFIG_HOME}" ]] ; then
	SGEBUILD_CONFIG_BASE_DIR="${HOME}/.config"
else
	SGEBUILD_CONFIG_BASE_DIR="${XDG_CONFIG_HOME}"
fi

SGEBUILD_CONFIG_DIR="${SGEBUILD_CONFIG_BASE_DIR}/sgebuild"


SGEBUILD_PROJECT_SOURCE_DIR="$(pwd)"

while [[ "${SGEBUILD_PROJECT_SOURCE_DIR}" != '/' && ! -d "${SGEBUILD_PROJECT_SOURCE_DIR}/.sgebuild" ]] ; do
	SGEBUILD_PROJECT_SOURCE_DIR="$(dirname "${SGEBUILD_PROJECT_SOURCE_DIR}")"
done

SGEBUILD_PROJECT_CONFIG_PREFIX=".sgebuild"
SGEBUILD_PROJECT_CONFIG_DIR="${SGEBUILD_PROJECT_SOURCE_DIR}/${SGEBUILD_PROJECT_CONFIG_PREFIX}"
