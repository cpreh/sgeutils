#!/bin/sh

find \( \( -name '*.?pp' -o -name '*.txt' -o -name '*.cmake' \) ! -regex '.*/\..*' \) -exec chmod '-x' '{}' \;
