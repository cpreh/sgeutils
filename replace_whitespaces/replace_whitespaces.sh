#!/bin/sh
find \( \
	-name '*.?pp' \
	-o -name '*.txt' \
	-o -name '*.sh' \
	-o -name '*.cmake' \
	-o -name '*.doxygen' \
	-o -name '*.html' \
	-o -name '*.xml' \
	-o -name '*.css' \
\) \
-type f -exec sed -i 's/[[:space:]]*$//' '{}' \;
