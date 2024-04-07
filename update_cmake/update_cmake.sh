#!/usr/bin/env bash

declare -a FIND_PARAMS

if [[ ! -v FIND_PATTERN ]]; then
    FIND_PARAMS=("-name" '*.?pp' "-o" "-name" '*.c' "-o" "-name" '*.h')
else
    FIND_PARAMS=("-name" "${FIND_PATTERN}")
fi

LC_COLLATE="C" find "${@:2}" "${FIND_PARAMS[@]}" | sort > "$1"
