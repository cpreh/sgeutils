#include <fcppt/filesystem/extension_without_dot.hpp>
#include <fcppt/filesystem/is_regular.hpp>
#include <fcppt/filesystem/path.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/filesystem/recursive_directory_iterator.hpp>
#include <fcppt/filesystem/remove_extension.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/io/ifstream.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/next_prior.hpp>
#include <iterator>
#include <string>
#include <cassert>
#include <cstdlib>

namespace
{

fcppt::filesystem::path::iterator::difference_type
path_levels(
	fcppt::filesystem::path const &_base_path
)
{
	fcppt::filesystem::path::iterator::difference_type ret(
		std::distance(
			_base_path.begin(),
			_base_path.end()
		)
	);

	if(
		ret > 0
		&&
		fcppt::filesystem::path_to_string(
			*boost::prior(
				_base_path.end()
			)
		).empty()
	)
		--ret;

	return ret;
}

fcppt::string const
make_include_guard(
	fcppt::filesystem::path::iterator::difference_type const _base_level,
	fcppt::filesystem::path const &_path
)
{
	fcppt::string ret;

	fcppt::filesystem::path const without_extension(
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
		fcppt::filesystem::path::const_iterator it(
			boost::next(
				without_extension.begin(),
				_base_level
			)
		);
		it != without_extension.end();
		++it
	)
		ret +=
			boost::algorithm::to_upper_copy(
				fcppt::filesystem::path_to_string(
					*it
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

}

int
main(
	int argc,
	char **argv
)
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

	fcppt::filesystem::path const base_path(
		fcppt::from_std_string(
			argv[1]
		)
	);

	fcppt::filesystem::path::iterator::difference_type const base_levels(
		::path_levels(
			base_path
		)
	);

	for(
		fcppt::filesystem::recursive_directory_iterator it(
			base_path
		), end;
		it != end;
		++it
	)
	{
		fcppt::filesystem::path const &path(
			it->path()
		);

		if(
			!fcppt::filesystem::is_regular(
				path
			)
		)
			continue;

		if(
			fcppt::filesystem::extension_without_dot(
				path
			)
			!=
			FCPPT_TEXT("hpp")
		)
			continue;

		fcppt::io::ifstream stream(
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
			!guard_found
		)
			fcppt::io::cout()
				<< path
				<< FCPPT_TEXT(": ")
				<< include_guard
				<< FCPPT_TEXT('\n');
	}
}
