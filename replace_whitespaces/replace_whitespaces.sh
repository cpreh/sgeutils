#!/bin/sh
find \( \
	\( \
		! -path './.sgebuild/*' \
		-a -type f \
	\) \
	-a  \
	\( \
	-name '*.?pp' \
	-o -name '*.txt' \
	-o -name '*.sh' \
	-o -name '*.cmake' \
	-o -name '*.doxygen' \
	-o -name '*.html' \
	-o -name '*.xml' \
	-o -name '*.css' \
	\) \
\) \
-exec grep -q '[[:space:]]\+$' '{}' \; \
-exec echo "Replacing " '{}' \; \
-exec sed -i 's/[[:space:]]*$//' '{}' \;
