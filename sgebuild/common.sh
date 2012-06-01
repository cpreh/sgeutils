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
	SGEBUILD_CONFIG_BASE_PATH="${HOME}/.config"
else
	SGEBUILD_CONFIG_BASE_PATH="${XDG_CONFIG_HOME}"
fi

SGEBUILD_CONFIG_PATH="${SGEBUILD_CONFIG_BASE_PATH}/sgebuild"

SGEBUILD_PROJECT_CONFIG_PATH=".sgebuild"
