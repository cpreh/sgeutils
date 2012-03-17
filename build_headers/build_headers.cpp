#include <fcppt/filesystem/extension.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/filesystem/remove_filename.hpp>
#include <fcppt/filesystem/replace_extension.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/io/ofstream.hpp>
#include <fcppt/preprocessor/disable_gcc_warning.hpp>
#include <fcppt/preprocessor/pop_warning.hpp>
#include <fcppt/preprocessor/push_warning.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/optional.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <fcppt/to_std_string.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <fcppt/config/external_end.hpp>

namespace
{

typedef fcppt::optional<
	boost::filesystem::path
> optional_path;

optional_path const
find_cmake_dir(
	boost::filesystem::path const &_path
)
{
	for(
		boost::filesystem::directory_iterator
			build_dir_it(
				_path
			),
			build_dir_end;
		build_dir_it != build_dir_end;
		++build_dir_it
	)
		if(
			boost::algorithm::ends_with(
				fcppt::filesystem::path_to_string(
					build_dir_it->path().filename()
				),
				fcppt::string(
					FCPPT_TEXT(".dir")
				)
			)
		)
			return
				optional_path(
					*build_dir_it
				);

	return
		optional_path();
}

bool
write_file(
	boost::filesystem::path const &_file,
	fcppt::string const &_content
)
{
	fcppt::io::ofstream ofs(
		_file
	);

	if(
		!ofs.is_open()
	)
	{
		fcppt::io::cerr()
			<< FCPPT_TEXT("Failed to open ")
			<< fcppt::filesystem::path_to_string(
				_file
			)
			<< FCPPT_TEXT('\n');

		return false;
	}

	ofs
		<< _content;

	if(
		!ofs
	)
	{
		fcppt::io::cerr()
			<< FCPPT_TEXT("Writing ")
			<< fcppt::filesystem::path_to_string(
				_file
			)
			<< FCPPT_TEXT(" failed\n");

		return false;
	}

	return true;
}

boost::filesystem::path const
make_path_from_iter(
	boost::filesystem::path::iterator _beg,
	boost::filesystem::path::iterator const _end
)
{
	boost::filesystem::path ret;

	for(
		;
		_beg != _end;
		++_beg
	)
		ret /= *_beg;

	return ret;
}

}

int
main(
	int argc,
	char **argv
)
{
	if(
		argc <= 0
	)
		return EXIT_FAILURE;

	if(
		argc != 2
		&& argc != 3
	)
	{
		fcppt::io::cerr()
			<< FCPPT_TEXT("Usage: ")
			<< fcppt::from_std_string(
				argv[0]
			)
			<< FCPPT_TEXT(" <build_dir> [src_dir]\n");

		return EXIT_FAILURE;
	}

	boost::filesystem::path const build_dir(
		fcppt::from_std_string(
			argv[1]
		)
	);

	boost::filesystem::path const temp_dir(
		build_dir
		/ FCPPT_TEXT("temp_make_headers")
	);

	boost::filesystem::path const make_file(
		temp_dir
		/ FCPPT_TEXT("Makefile")
	);

	boost::filesystem::path const log_file(
		temp_dir
		/ FCPPT_TEXT("log.txt")
	);

	boost::filesystem::remove(
		log_file
	);

	fcppt::string const source_subdir(
		argc == 3
		?
			fcppt::from_std_string(
				argv[2]
			)
		:
			FCPPT_TEXT("src")
	);

	if(
		!boost::filesystem::exists(
			temp_dir
		)
		&&
		!boost::filesystem::create_directory(
			temp_dir
		)
	)
	{
		fcppt::io::cerr()
			<< FCPPT_TEXT("Cannot create ")
			<< temp_dir
			<< FCPPT_TEXT('\n');

		return EXIT_FAILURE;
	}

	for(
		boost::filesystem::recursive_directory_iterator
			dir_it(
				FCPPT_TEXT("include")
			),
			end_it;
		dir_it != end_it;
		++dir_it
	)
	{
		boost::filesystem::path const &path(
			dir_it->path()
		);

		if(
			fcppt::filesystem::extension(
				path
			)
			!= FCPPT_TEXT(".hpp")
		)
			continue;

		if(
			std::find(
				path.begin(),
				path.end(),
				fcppt::string(
					FCPPT_TEXT("impl")
				)
			)
			!= path.end()
		)
			continue;

		boost::filesystem::path::iterator path_it(
			path.begin()
		);

		if(
			path_it
			== path.end()
		)
			continue;

		// skip include/
		++path_it;

		if(
			path_it
			== path.end()
		)
			continue;

		boost::filesystem::path const include_file(
			::make_path_from_iter(
				path_it,
				path.end()
			)
		);

		// skip project name
		++path_it;

		if(
			path_it
			== path.end()
		)
			continue;

		// descend into the build dir as far as possible
		boost::filesystem::path make_path(
			build_dir
			/
			source_subdir
		);

		while(
			boost::filesystem::exists(
				make_path
			)
		)
			make_path /= *path_it++;

		make_path =
			fcppt::filesystem::remove_filename(
				make_path
			)
			/
			FCPPT_TEXT("CMakeFiles");

		if(
			!boost::filesystem::exists(
				make_path
			)
		)
		{
			fcppt::io::cerr()
				<< make_path
				<< FCPPT_TEXT(" does not exist.\n");

			continue;
		}

		optional_path const cmake_dir(
			::find_cmake_dir(
				make_path
			)
		);

		if(
			!cmake_dir
		)
		{
			fcppt::io::cerr()
				<< FCPPT_TEXT("No *.dir found in ")
				<< make_path
				<< FCPPT_TEXT('\n');

			continue;
		}

		boost::filesystem::path const flags_file(
			*cmake_dir
			/ FCPPT_TEXT("flags.make")
		);

		if(
			!boost::filesystem::exists(
				flags_file
			)
		)
		{
			fcppt::io::cerr()
				<< flags_file
				<< FCPPT_TEXT(" does not exist!\n");

			continue;
		}

		boost::filesystem::path const source_file(
			temp_dir
			/
			boost::algorithm::replace_all_copy(
				fcppt::filesystem::path_to_string(
					fcppt::filesystem::replace_extension(
						include_file,
						FCPPT_TEXT("cpp")
					)
				),
				FCPPT_TEXT("/"),
				FCPPT_TEXT("_")
			)
		);

		if(
			!::write_file(
				make_file,
				FCPPT_TEXT("include ")
				+ fcppt::filesystem::path_to_string(
					flags_file
				)
				+ FCPPT_TEXT("\n\n")
				+ FCPPT_TEXT("check-syntax:\n")
				+ FCPPT_TEXT("\tg++ -o /dev/null ${CXX_FLAGS} ${CXX_DEFINES} -S ")
				+ fcppt::filesystem::path_to_string(
					source_file
				)
				+ FCPPT_TEXT(" -fsyntax-only\n\n")
				+ FCPPT_TEXT(".PHONY: check-syntax*/\n")
			)
		)
			return EXIT_FAILURE;

		if(
			!::write_file(
				source_file,
				FCPPT_TEXT("#include <")
				+ fcppt::filesystem::path_to_string(
					include_file
				)
				+ FCPPT_TEXT(">\n")
			)
		)
			return EXIT_FAILURE;

		int const system_ret(
			std::system(
				(
					std::string(
						"make -f "
					)
					+
					fcppt::to_std_string(
						fcppt::filesystem::path_to_string(
							make_file
						)
					)
					+
					" 2>>"
					+
					fcppt::to_std_string(
						fcppt::filesystem::path_to_string(
							log_file
						)
					)
					+
					" 1>/dev/null"
				).c_str()
			)
		);

FCPPT_PP_PUSH_WARNING
FCPPT_PP_DISABLE_GCC_WARNING(-Wcast-qual)
FCPPT_PP_DISABLE_GCC_WARNING(-Wold-style-cast)
		if(
			system_ret == -1
			||
			(
				WIFSIGNALED(
					system_ret
				)
				&&
				(
					WTERMSIG(
						system_ret
					)
					== SIGINT
					||
					WTERMSIG(
						system_ret
					)
					== SIGQUIT
				)
			)
		)
			return EXIT_FAILURE;
FCPPT_PP_POP_WARNING
	}
}
