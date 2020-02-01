#include <fcppt/char_type.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <fcppt/filesystem/extension_without_dot.hpp>
#include <fcppt/filesystem/normalize.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/filesystem/remove_extension.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <fcppt/config/external_end.hpp>


namespace
{

std::filesystem::path::iterator::difference_type
path_levels(
	std::filesystem::path const &_base_path
)
{
	std::filesystem::path::iterator::difference_type ret(
		std::distance(
			_base_path.begin(),
			_base_path.end()
		)
	);

	if(
		ret > 0
		&&
		fcppt::filesystem::path_to_string(
			*std::prev(
				_base_path.end()
			)
		).empty()
	)
		--ret;

	return ret;
}

fcppt::string
make_include_guard(
	std::filesystem::path::iterator::difference_type const _base_level,
	std::filesystem::path const &_path
)
{
	fcppt::string ret;

	std::filesystem::path const without_extension(
		fcppt::filesystem::remove_extension(
			_path
		)
	);

	assert(
		std::distance(
			_path.begin(),
			_path.end()
		)
		>=
		_base_level
	);

	for(
		std::filesystem::path const &path
		:
		boost::make_iterator_range(
			std::next(
				without_extension.begin(),
				_base_level
			),
			without_extension.end()
		)
	)
		ret +=
			boost::algorithm::to_upper_copy(
				fcppt::filesystem::path_to_string(
					path
				)
			)
			+ FCPPT_TEXT('_');

	return
		ret
		+
		boost::algorithm::to_upper_copy(
			fcppt::filesystem::extension_without_dot(
				_path
			)
		)
		+
		FCPPT_TEXT("_INCLUDED");
}

bool
is_reserved_identifier(
	fcppt::string const &_identifier
)
{
	return
		!_identifier.empty()
		&&
		(
			_identifier[
				0
			]
			==
			FCPPT_TEXT('_')
			||
			_identifier.find(
				FCPPT_TEXT("__")
			)
			!=
			fcppt::string::npos
		);
}

}

int
main(
	int argc,
	char **argv
)
try
{
	if(
		argc < 2
		|| argc > 3
	)
	{
		fcppt::io::cerr()
			<< FCPPT_TEXT("Usage: ")
			<< argv[0]
			<< FCPPT_TEXT(" path [prefix]\n");

		return EXIT_FAILURE;
	}

	fcppt::string const prefix(
		argc == 3
		?
			fcppt::from_std_string(
				argv[2]
			)
		:
			fcppt::string()
	);

	std::filesystem::path const base_path(
		fcppt::filesystem::normalize(
			std::filesystem::path(
				fcppt::from_std_string(
					argv[1]
				)
			)
		)
	);

	std::filesystem::path::iterator::difference_type const base_levels(
		::path_levels(
			base_path
		)
	);

	for(
		std::filesystem::path const &path
		:
		boost::make_iterator_range(
			std::filesystem::recursive_directory_iterator(
				base_path
			),
			std::filesystem::recursive_directory_iterator()
		)
	)
	{
		if(
			!std::filesystem::is_regular_file(
				path
			)
		)
			continue;

		fcppt::string const extension(
			fcppt::filesystem::extension_without_dot(
				path
			)
		);

		if(
			extension
			!=
			FCPPT_TEXT("hpp")
			&&
			extension
			!=
			FCPPT_TEXT("h")
		)
			continue;

		std::basic_ifstream<
			fcppt::char_type
		> stream(
			path
		);

		if(
			!stream.is_open()
		)
		{
			fcppt::io::cerr()
				<< FCPPT_TEXT("Failed to open ")
				<< path
				<< FCPPT_TEXT('\n');

			continue;
		}

		fcppt::string const include_guard(
			prefix
			+
			::make_include_guard(
				base_levels,
				path
			)
		);

		fcppt::string const ifndef_line(
			FCPPT_TEXT("#ifndef ")
			+ include_guard
		);

		fcppt::string const define_line(
			FCPPT_TEXT("#define ")
			+ include_guard
		);

		fcppt::string line;

		bool guard_found = false;

		while(
			std::getline(
				stream,
				line
			)
		)
		{
			fcppt::string line2;

			if(
				line
				==
				ifndef_line
				&&
				(
					std::getline(
						stream,
						line2
					)
					&&
					line2
					==
					define_line
				)
			)
			{
				guard_found = true;

				break;
			}
		}

		if(
			is_reserved_identifier(
				include_guard
			)
		)
			fcppt::io::cout()
				<< path
				<< FCPPT_TEXT(": Include guard ")
				<< include_guard
				<< FCPPT_TEXT(" is a reserved identifier.\n");

		if(
			!guard_found
		)
			fcppt::io::cout()
				<< path
				<< FCPPT_TEXT(": ")
				<< include_guard
				<< FCPPT_TEXT('\n');
	}
}
catch(
	fcppt::exception const &_error
)
{
	fcppt::io::cerr()
		<<
		_error.string()
		<<
		FCPPT_TEXT('\n');
}
catch(
	std::exception const &_error
)
{
	std::cerr
		<<
		_error.what()
		<<
		'\n';
}
