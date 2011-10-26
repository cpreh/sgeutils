#!/bin/sh

find \( \( -name '*.?pp' -o -name '*.txt' \) ! -regex '.*/\..*' \) -exec chmod '-x' '{}' \;
