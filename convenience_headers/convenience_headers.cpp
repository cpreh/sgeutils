#include <fcppt/algorithm/contains.hpp>
#include <fcppt/assign/make_container.hpp>
#include <fcppt/filesystem/directory_iterator.hpp>
#include <fcppt/filesystem/is_directory.hpp>
#include <fcppt/filesystem/path.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/filesystem/recursive_directory_iterator.hpp>
#include <fcppt/filesystem/remove_extension.hpp>
#include <fcppt/filesystem/stem.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/ofstream.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>
#include <algorithm>
#include <exception>
#include <iosfwd>
#include <iostream>
#include <ostream>
#include <vector>
#include <cassert>
#include <cstdlib>

namespace
{

fcppt::filesystem::path const
make_header(
	fcppt::filesystem::path const &_path
)
{
	assert(
		fcppt::filesystem::is_directory(
			_path
		)
	);

	return
		_path
		/
		(
			fcppt::filesystem::stem(
				_path
			)
			+ FCPPT_TEXT(".hpp")
		);

}

fcppt::string const
make_include_guard(
	fcppt::filesystem::path const &_path
)
{
	fcppt::string ret;

	fcppt::filesystem::path const new_path(
		fcppt::filesystem::remove_extension(
			_path
		)
	);

	for(
		fcppt::filesystem::path::const_iterator it(
			new_path.begin()
		);
		it != new_path.end();
		++it
	)
	{
		fcppt::string component(
			fcppt::filesystem::path_to_string(
				*it
			)
		);

		boost::algorithm::to_upper(
			component
		);

		ret +=
			component
			+ FCPPT_TEXT('_');
	}

	ret += FCPPT_TEXT("HPP_INCLUDED");

	return ret;
}

typedef std::vector<
	fcppt::string
> string_vector;

string_vector const exclusions(
	fcppt::assign::make_container<
		string_vector
	>(
		FCPPT_TEXT("detail")
	)
	(
		FCPPT_TEXT("impl")
	)
);

bool
needs_header(
	fcppt::filesystem::path const &_path
)
{
	return
		fcppt::filesystem::is_directory(
			_path
		)
		&&
		!fcppt::algorithm::contains(
			exclusions,
			fcppt::filesystem::stem(
				_path
			)
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
		argc != 2
	)
	{
		fcppt::io::cerr
			<< FCPPT_TEXT("Usage: ")
			<< argv[0]
			<< FCPPT_TEXT(" <dir1>\n");

		return EXIT_FAILURE;
	}

	fcppt::filesystem::path const dir(
		fcppt::from_std_string(
			argv[1]
		)
	);

	for(
		fcppt::filesystem::recursive_directory_iterator dir_it(
			dir
		),
		dir_end;
		dir_it != dir_end;
		++dir_it
	)
	{
		if(
			!needs_header(
				dir_it->path()
			)
		)
			continue;

		fcppt::filesystem::path const header(
			make_header(
				dir_it->path()
			)
		);

		fcppt::io::ofstream file(
			header,
			std::ios_base::trunc
		);

		string_vector filenames;

		if(
			!file.is_open()
		)
		{
			fcppt::io::cerr
				<< FCPPT_TEXT("Failed to open ")
				<< header
				<< FCPPT_TEXT('\n');

			return EXIT_FAILURE;
		}

		for(
			fcppt::filesystem::directory_iterator file_it(
				dir_it->path()
			),
			file_end;
			file_it != file_end;
			++file_it
		)
		{
			if(
				file_it->path()
				== header
			)
				continue;

			if(
				needs_header(
					file_it->path()
				)
			)
				filenames.push_back(
					fcppt::filesystem::path_to_string(
						make_header(
							file_it->path()
						)
					)
				);
			else if(
				!fcppt::filesystem::is_directory(
					file_it->path()
				)
			)
				filenames.push_back(
					fcppt::filesystem::path_to_string(
						file_it->path()
					)
				);
		}

		std::sort(
			filenames.begin(),
			filenames.end()
		);

		fcppt::string const include_guard_name(
			make_include_guard(
				header
			)
		);

		file
			<< FCPPT_TEXT("#ifndef ")
			<< include_guard_name
			<< FCPPT_TEXT("\n#define ")
			<< include_guard_name
			<< FCPPT_TEXT("\n\n");

		BOOST_FOREACH(
			string_vector::const_reference cur_name,
			filenames
		)
			file
				<< FCPPT_TEXT("#include <")
				<< cur_name
				<< FCPPT_TEXT(">\n");

		file
			<< FCPPT_TEXT("\n#endif\n");
	}
}
catch(
	fcppt::exception const &_error
)
{
	fcppt::io::cerr
		<< _error.string()
		<< FCPPT_TEXT('\n');
}
catch(
	std::exception const &_error
)
{
	std::cerr
		<< _error.what()
		<< '\n';
}
