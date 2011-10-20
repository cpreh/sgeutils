#!/usr/bin/python3

import sys
import re
import argparse

# Helper functions
def remove_duplicates_from_list(mylist,key):
	"""
	Remove duplicate entries (based on the functor "key") from the
	list *IN PLACE*
	"""
	last = mylist[-1]
	for i in range(len(mylist)-2, -1, -1):
		if last == key(mylist[i]):
			del mylist[i]
		else:
			last = key(mylist[i])
	return mylist

# The main function
def modify_file(filename,reserved_prefixes,debug):
	lines = []
	with open(filename,'r') as f:
		lines = [l.rstrip() for l in f.readlines()]

	begin_of_includes = None
	end_of_includes = None

	# Helper struct, holds all relevant information about an include line
	class IncludeLine:
		def __init__(
			self,
			path,
			prefix,
			rest):

			self.prefix = prefix
			self.path = path
			self.rest = rest

		def __repr__(
			self):

			return self.path.__repr__()

	original_raw_include_lines = []
	include_lines = []

	compiled_include_regex = re.compile(r'\s*#include\s+<(([^/]+)(/[^>]*)?)>')

	line_counter = 0
	for line in lines:
		include_search_result = compiled_include_regex.search(line)

		if include_search_result != None:
			original_raw_include_lines.append(
				line)

			# We've found the starting include!
			if begin_of_includes == None:
				begin_of_includes = line_counter
			else:
				# We found the start _and_ the end and then another #include line!
				if end_of_includes != None:
					sys.stderr.write('Error: The file {} has non-continuous includes.\nBegin of includes was line {}, end of includes was line {}.\nLine {}:\n\n{}\n\n...re-enters an include chain. Cannot continue...\n'.format(filename,begin_of_includes,end_of_includes,line_counter,line))
					return False

			prefix = include_search_result.group(2)
			rest = include_search_result.group(3)
			path = include_search_result.group(1)

			if prefix != 'fcppt' or (rest != '/config/external_begin.hpp' and rest != '/config/external_end.hpp'):
				include_lines.append(
					IncludeLine(
						path = path,
						prefix = prefix,
						rest = rest))
		else:
			# A non-#include-line. If we've found the start, then the only allowed
			# lines are blank lines
			if begin_of_includes != None and re.search('^\s*$',line) == None and end_of_includes == None:
				end_of_includes = line_counter

		line_counter += 1

	if debug == True:
		print('Begin of includes: {}, end of includes: {}'.format(begin_of_includes,end_of_includes))

	# There might be files without any <> includes
	if begin_of_includes == None and end_of_includes == None:
		return True

	assert begin_of_includes != None and end_of_includes != None

	groups = {}

	for l in include_lines:
		corrected_prefix = l.prefix

		if not corrected_prefix in reserved_prefixes:
			corrected_prefix = ''

		if not corrected_prefix in groups:
			groups[corrected_prefix] = [l]
		else:
			groups[corrected_prefix].append(
				l)

	for name,includes in groups.items():
		includes.sort(
			key = lambda x : x.path)

		remove_duplicates_from_list(
			includes,
			lambda x : x.path)

	new_includes = []

	for prefix in reserved_prefixes:
		if prefix not in groups:
			continue
		new_includes += list(
				map(
					lambda x : '#include <{}>'.format(x.path),
					groups[prefix]))

	if '' in groups and len(groups['']) != 0:
		new_includes.append('#include <fcppt/config/external_begin.hpp>')
		new_includes += list(
				map(
					lambda x : '#include <{}>'.format(x.path),
					groups['']))
		new_includes.append('#include <fcppt/config/external_end.hpp>')


	modifications_present = new_includes != original_raw_include_lines
	new_includes.append('\n')
	lines[begin_of_includes:end_of_includes] = new_includes

	if debug == True:
		if modifications_present == False:
			print('No modification to {}'.format(filename))
		else:
			# Remove old includes and add the new ones
			print('Modification needed: ')
			for l in lines:
				sys.stdout.write(
					l+'\n')

	else:
		if modifications_present == True:
			with open(filename,'w') as f:
				f.write(("\n".join(lines))+'\n')

# Begin of program
parser = argparse.ArgumentParser(
	description = 'Clean up blocks of #include statements')

parser.add_argument(
	'--reserved-prefix',
	# This creates a list instead of a single value (e.g. --reserved-prefix a --reserved-prefix b => [a,b]
	action = 'append')

parser.add_argument(
	'--debug',
	action='store_true')

parser.add_argument(
	'files',
	nargs = '+')

parser_result = parser.parse_args(
	args = sys.argv[1:])

reserved_prefixes = parser_result.reserved_prefix if parser_result.reserved_prefix != None else []

erroneous_files = []

for filename in parser_result.files:
	if parser_result.debug == True:
		print('Looking at file {}'.format(filename))

	if modify_file(filename,reserved_prefixes,parser_result.debug) == False:
		erroneous_files.append(
			filename)

print('The following files encountered errors:')
for f in erroneous_files:
	print(f)
