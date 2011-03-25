#include <boost/filesystem/v3/convenience.hpp>
#include <boost/filesystem/v3/operations.hpp>
#include <boost/filesystem/v3/path.hpp>
#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cstdlib>

namespace
{

typedef std::vector<
	std::string
> file_vector;

template<
	typename Iterator
>
void
add_files(
	Iterator _iterator,
	file_vector &_files
)
{
	for(
		;
		_iterator != Iterator();
		++_iterator
	)
	{
		if(
			!boost::filesystem::is_regular(
				*_iterator
			)
		)
			continue;

		std::string const extension(
			boost::filesystem::extension(
				*_iterator
			)
		);

		if(
			extension.size()
			!= 4u
			||
			extension.substr(
				2u,
				2u
			)
			!= "pp"
		)
			continue;

		_files.push_back(
			_iterator->path().string()
		);
	}
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
		argc <= 3
	)
	{
		std::cerr
			<< "Usage: "
			<< argv[0]
			<< " <CMakeFile> <VAR_NAME> <path1> [path2] ...\n"
			<< "In front of every path the additional option -r or -n is accepted.\n"
			<< "-r will search recursively, while -n will not. The default is -r.\n";

		return EXIT_FAILURE;
	}

	std::string const cmake_file(
		argv[1]
	);

	std::ifstream ifs(
		cmake_file.c_str()
	);

	if(
		!ifs.is_open()
	)
	{
		std::cerr
			<< cmake_file
			<< " does not exist.\n";

		return EXIT_FAILURE;
	}

	std::string const out_file(
		cmake_file
		+ ".new"
	);

	std::ofstream ofs(
		out_file.c_str()
	);

	if(
		!ofs.is_open()
	)
	{
		std::cerr
			<< "Cannot open "
			<< out_file
			<< '\n';

		return EXIT_FAILURE;
	}

	file_vector files;

	std::string mode(
		"r"
	);

	for(
		int arg = 3;
		arg < argc;
		++arg
	)
	{
		std::string const arg_string(
			argv[arg]
		);

		if(
			!arg_string.empty()
			&&
			arg_string[0] == '-'
		)
		{
			mode =
				arg_string.substr(
					1u
				);

			continue;
		}
		
		if(
			mode == "r"
		)
			::add_files(
				boost::filesystem::recursive_directory_iterator(
					arg_string
				),
				files
			);
		else if(
			mode == "n"
		)
			::add_files(
				boost::filesystem::directory_iterator(
					arg_string
				),
				files
			);
		else
		{
			std::cerr
				<< "Invalid mode "
				<< mode
				<< '\n';

			return EXIT_FAILURE;
		}
	}

	std::sort(
		files.begin(),
		files.end()
	);

	std::string const search_line(
		std::string(
			"\t"
		)
		+ argv[2]
	);

	std::string line;

	while(
		std::getline(
			ifs,
			line
		)
		&&
		line
		!= search_line
	)
		ofs
			<< line
			<< '\n';

	if(
		line != search_line
		)
	{
		std::cerr
			<< search_line
			<< " not found!\n";

		return EXIT_FAILURE;
	}

	ofs
		<< line
		<< '\n';

	for(
		file_vector::const_iterator it(
			files.begin()
		);
		it != files.end();
		++it
	)
		ofs
			<< '\t'
			<< *it
			<< '\n';

	std::string const search_end(
		")"
	);

	while(
		std::getline(
			ifs,
			line
		)
		&&
		line
		!= search_end
	) ;

	if(
		line != search_end
	)
	{
		std::cerr
			<< search_end
			<< " not found!\n";

		return EXIT_FAILURE;
	}

	ofs
		<< line
		<< '\n';

	while(
		std::getline(
			ifs,
			line
		)
	)
		ofs
			<< line
			<< '\n';

	return EXIT_SUCCESS;
}
catch(
	std::exception const &_error
)
{
	std::cerr
		<< _error.what()
		<< '\n';
	
	return EXIT_FAILURE;
}
