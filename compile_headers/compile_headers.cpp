#include <sge/parse/json/array.hpp>
#include <sge/parse/json/element_vector.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/parse_file_exn.hpp>
#include <sge/parse/json/start.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/extract_from_string.hpp>
#include <fcppt/optional_impl.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <fcppt/to_std_string.hpp>
#include <fcppt/algorithm/contains.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/preprocessor/disable_gcc_warning.hpp>
#include <fcppt/preprocessor/pop_warning.hpp>
#include <fcppt/preprocessor/push_warning.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/filesystem/path.hpp>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <ostream>
#include <thread>
#include <vector>
#include <fcppt/config/external_end.hpp>


namespace
{

fcppt::string
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
				std::next(
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

void
worker(
	sge::parse::json::value const &_element,
	boost::asio::io_service &_io_service
)
{
	sge::parse::json::object const &json_object(
		_element.get<
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
			return;
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

		_io_service.stop();

		return;
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

		_io_service.stop();
	}
}

unsigned
extract_threads(
	int const _argc,
	char const *const *const _argv
)
{
	if(
		_argc != 2
	)
		return
			std::max(
				std::thread::hardware_concurrency(),
				1u
			);

	std::string const arg(
		_argv[1]
	);

	if(
		arg.size() < 2u
		||
		arg.substr(
			0u,
			2u
		)
		!=
		"-j"
	)
		throw
			fcppt::exception(
				FCPPT_TEXT("Invalid parameter!")
			);

	typedef
	fcppt::optional<
		unsigned
	>
	unsigned_optional;

	unsigned_optional const result(
		fcppt::extract_from_string<
			unsigned
		>(
			arg.substr(
				2u
			)
		)
	);

	if(
		!result
	)
		throw fcppt::exception(
			FCPPT_TEXT("-j takes an int parameter")
		);

	return
		*result;
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
		argc > 2
	)
	{
		fcppt::io::cerr()
			<< FCPPT_TEXT("This command takes no parameters or a -j parameter!\n");

		return EXIT_FAILURE;
	}

	unsigned const num_threads(
		extract_threads(
			argc,
			argv
		)
	);

	fcppt::io::cout() << FCPPT_TEXT("Starting ") << num_threads << FCPPT_TEXT(" threads\n");

	sge::parse::json::array const build_commands(
		sge::parse::json::parse_file_exn(
			FCPPT_TEXT("compile_commands.json")
		).array()
	);

	boost::asio::io_service io_service;

	for(
		auto const &element : build_commands.elements
	)
	{
		io_service.post(
			std::bind(
				worker,
				element,
				std::ref(
					io_service
				)
			)
		);
	}

	typedef
	std::vector<
		std::thread
	>
	thread_vector;

	thread_vector threads;

	for(
		unsigned index = 0;
		index < num_threads;
		++index
	)
		threads.push_back(
			std::thread(
				[
					&io_service
				]()
				{
					io_service.run();
				}
			)
		);

	for(
		auto &thread : threads
	)
		thread.join();

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
