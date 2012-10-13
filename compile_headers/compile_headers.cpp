#include <fcppt/exception.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <fcppt/to_std_string.hpp>
#include <fcppt/algorithm/contains.hpp>
#include <fcppt/error/strerrno.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/preprocessor/disable_gcc_warning.hpp>
#include <fcppt/preprocessor/pop_warning.hpp>
#include <fcppt/preprocessor/push_warning.hpp>
#include <sge/parse/json/array.hpp>
#include <sge/parse/json/element_vector.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/parse_file_exn.hpp>
#include <sge/parse/json/start.hpp>
#include <fcppt/config/external_begin.hpp>
#include <unistd.h>
#include <boost/next_prior.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/path.hpp>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <fcppt/config/external_end.hpp>


namespace
{

fcppt::string const
make_syntax_only(
	fcppt::string const &_compile_command
)
{
	typedef
	std::vector<
		fcppt::string
	>
	string_sequence;

	string_sequence parts;

	boost::algorithm::split(
		parts,
		_compile_command,
		boost::algorithm::is_space(),
		boost::algorithm::token_compress_on
	);

	for(
		string_sequence::iterator it(
			parts.begin()
		);
		it != parts.end();
	)
	{
		if(
			*it == FCPPT_TEXT("-o")
		)
		{
			if(
				boost::next(
					it
				)
				==
				parts.end()
			)
				throw fcppt::exception(
					FCPPT_TEXT("Nothing following a -o argument!")
				);

			it =
				parts.erase(
					it,
					it + 2
				);
		}
		else
		{
			++it;
		}
	}

	parts.push_back(
		FCPPT_TEXT("-fsyntax-only")
	);

	return
		boost::algorithm::join(
			parts,
			fcppt::string(
				FCPPT_TEXT(" ")
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
		fcppt::io::cerr()
			<< FCPPT_TEXT("You need to pass the relative build directory as first parameter!\n");

		return EXIT_FAILURE;
	}

	{
		std::string const build_dir(
			argv[1]
		);

		if(
			::chdir(
				build_dir.c_str()
			)
			!= 0
		)
		{
			fcppt::io::cerr()
				<< "Unable to change directory: "
				<< fcppt::error::strerrno();

			return EXIT_FAILURE;
		}
	}

	sge::parse::json::array const build_commands(
		sge::parse::json::parse_file_exn(
			FCPPT_TEXT("compile_commands.json")
		).array()
	);

	for(
		sge::parse::json::element_vector::const_iterator it(
			build_commands.elements.begin()
		);
		it != build_commands.elements.end();
		++it
	)
	{
		sge::parse::json::object const &json_object(
			it->get<
				sge::parse::json::object
			>()
		);

		{
			boost::filesystem::path const filename(
				sge::parse::json::find_member_exn<
					sge::parse::json::string const
				>(
					json_object.members,
					fcppt::string(
						FCPPT_TEXT("file")
					)
				)
			);

			if(
				fcppt::algorithm::contains(
					filename,
					boost::filesystem::path(
						FCPPT_TEXT("impl")
					)
				)
				||
				filename.extension()
				!=
				".hpp"
			)
				continue;
		}

		int const exit_status(
			std::system(
				fcppt::to_std_string(
					make_syntax_only(
						sge::parse::json::find_member_exn<
							sge::parse::json::string const
						>(
							json_object.members,
							fcppt::string(
								FCPPT_TEXT("command")
							)
						)
					)
				).c_str()
			)
		);


		if(
			exit_status == -1
		)
		{
			fcppt::io::cerr()
				<< FCPPT_TEXT("system() failed\n");

			return EXIT_FAILURE;
		}

FCPPT_PP_PUSH_WARNING
FCPPT_PP_DISABLE_GCC_WARNING(-Wcast-qual)
FCPPT_PP_DISABLE_GCC_WARNING(-Wold-style-cast)
		if(
			WIFSIGNALED(
				exit_status
			)
		)
FCPPT_PP_POP_WARNING
		{
			fcppt::io::cout()
				<< FCPPT_TEXT('\n');

			break;
		}
	}

	return EXIT_SUCCESS;
}
catch(
	fcppt::exception const &_exception
)
{
	fcppt::io::cerr()
		<< _exception.string()
		<< FCPPT_TEXT('\n');

	return EXIT_FAILURE;
}
catch(
	std::exception const &_exception
)
{
	std::cerr
		<< _exception.what()
		<< '\n';

	return EXIT_FAILURE;
}
