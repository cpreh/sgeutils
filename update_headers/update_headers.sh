#!/usr/bin/env bash

set -e -u

if [[ "$#" -eq 0 ]] ; then
	echo "update_headers <filename> [other args for smartincludes]"
	exit 1
fi

orig="$1"
input=$(<"$orig")
output="$(echo "$input" | smartincludes --external-header-pair fcppt/config/external_begin.hpp,fcppt/config/external_end.hpp "${@:2}")"

if [[ "$input" != "$output" ]] ; then
	echo "$output" > "$orig"
fi
