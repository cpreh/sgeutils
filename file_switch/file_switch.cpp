#include <fcppt/args_char.hpp>
#include <fcppt/args_from_second.hpp>
#include <fcppt/main.hpp>
#include <fcppt/text.hpp>
#include <fcppt/assert/error_message.hpp>
#include <fcppt/either/match.hpp>
#include <fcppt/options/apply.hpp>
#include <fcppt/options/argument.hpp>
#include <fcppt/options/error.hpp>
#include <fcppt/options/error_output.hpp>
#include <fcppt/options/long_name.hpp>
#include <fcppt/options/optional_help_text.hpp>
#include <fcppt/options/optional_short_name.hpp>
#include <fcppt/options/parse.hpp>
#include <fcppt/options/result_of.hpp>
#include <fcppt/options/switch.hpp>
#include <fcppt/record/get.hpp>
#include <fcppt/record/make_label.hpp>
#include <fcppt/config/external_begin.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <string>
#include <fcppt/config/external_end.hpp>

namespace
{
/*
  Example:

  /home/philipp/programming/fcppt/include/fcppt/bar/baz.hpp

  Remove the filename part:

  /home/philipp/programming/fcppt/include/fcppt/bar/

  Go up, check for src/ and include/ below the target
  directory. Collect (in a vector) the elements removed.

  /home/philipp/programming/fcppt/include/fcppt/bar/

  Not found, go up

  /home/philipp/programming/fcppt/include/fcppt/
  /home/philipp/programming/fcppt/include/
  /home/philipp/programming/fcppt/

  Found. In our case, the elements removed are:

  include, fcppt, bar

  Since the last removed element is include, we remove it and get

  fcppt, bar

  Go to include/ from there and check if there's one directory
  there. That's the namespace name.

  Next, check if we've got a hpp or cpp file.

  If it's a cpp file, go to include/${namespace_name}/bar/ and look for the corresponding hpp file.
  If it's a hpp file, go to src/bar and look for the corresponding cpp file.

 */

int
main_program(
	std::filesystem::path const &_input_file_with_extension,
	bool const _debug_mode
)
{
	std::filesystem::path input_directory{
		_input_file_with_extension.parent_path()
	};

	std::filesystem::path namespace_path{};

	if(_debug_mode)
	{
		std::cout << "input_file_with_extension: " << _input_file_with_extension << "\n";
		std::cout << "input_directory: " << input_directory << "\n";
	}

	while(
		!std::filesystem::is_directory(input_directory / "src") || // NOLINT(fuchsia-default-arguments-calls)
		!std::filesystem::is_directory(input_directory / "include")) // NOLINT(fuchsia-default-arguments-calls)
	{
		namespace_path = *(std::prev(input_directory.end())) / namespace_path; // NOLINT(fuchsia-default-arguments-calls)
		input_directory = input_directory.parent_path();

		if(input_directory.empty())
		{
			std::cerr << "error: no directory found where include/ and src/ lie below. Exiting...\n";
			return EXIT_FAILURE;
		}

		if(_debug_mode)
		{
			std::cout << "input_directory: " << input_directory << "\n";
			std::cout << "namespace path: " << namespace_path << "\n";
		}
	}

	FCPPT_ASSERT_ERROR_MESSAGE(
		!namespace_path.empty(),
		FCPPT_TEXT("The namespace path is empty, so the file isn't under include/ or src/.\n")
		FCPPT_TEXT("Not sure what to do here..."));

	// I'm not sure if this is a good test.
	bool const is_header = namespace_path.begin()->string<char>() == "include"; // NOLINT(fuchsia-default-arguments-calls)

	if(is_header && _input_file_with_extension.extension().string<char>() != ".hpp") // NOLINT(fuchsia-default-arguments-calls)
	{
		std::cerr << "Got a file in include/ which doesn't end in \"hpp\" (extension is " << _input_file_with_extension.extension().string<char>() << ").\n"; // NOLINT(fuchsia-default-arguments-calls)
		std::cerr << "Not sure what to do, exiting...\n";
		return EXIT_FAILURE;
	}

	if(!is_header && _input_file_with_extension.extension().string<char>() != ".cpp") // NOLINT(fuchsia-default-arguments-calls)
	{
		std::cerr << "Got a file in src/ which doesn't end in \"cpp\". Not sure what to do, exiting...\n";
		return EXIT_FAILURE;
	}

	FCPPT_ASSERT_ERROR_MESSAGE(
		!is_header || std::distance(namespace_path.begin(),namespace_path.end()) > 1,
		FCPPT_TEXT("There's no directory below include/ denoting the namespace of the project. Don't know how to handle that yet."));

	std::filesystem::path const trimmed_namespace_path =
		std::accumulate(
			std::next(
				namespace_path.begin(),
				is_header
				?
					2
				:
					1),
			namespace_path.end(),
			std::filesystem::path(),
			std::divides<>());

	if(_debug_mode)
	{
		std::cout << "Trimmed namespace path: " << trimmed_namespace_path << "\n";
	}

	std::filesystem::directory_iterator include_directory_iterator(
		input_directory / "include"); // NOLINT(fuchsia-default-arguments-calls)

	if(include_directory_iterator == std::filesystem::directory_iterator())
	{
		std::cerr << "The include directory " << (input_directory / "include") << " is empty. Exiting...\n"; // NOLINT(fuchsia-default-arguments-calls)
		return EXIT_FAILURE;
	}

	std::string const namespace_name(
		include_directory_iterator->path().stem().string<char>()); // NOLINT(fuchsia-default-arguments-calls)

	if(_debug_mode)
	{
		std::cout << "Namespace name: " << namespace_name << "\n";
	}

	if(is_header)
	{
		std::cout << (input_directory / "src" / trimmed_namespace_path / _input_file_with_extension.stem().replace_extension(".cpp")).string<char>(); // NOLINT(fuchsia-default-arguments-calls)
	}
	else
	{
		std::cout << (input_directory / "include" / namespace_name / trimmed_namespace_path / _input_file_with_extension.stem().replace_extension(".hpp")).string<char>(); // NOLINT(fuchsia-default-arguments-calls)
	}

	return EXIT_SUCCESS;
}

}

int
FCPPT_MAIN(
	int argc,
	fcppt::args_char **argv
)
try
{
	FCPPT_RECORD_MAKE_LABEL(
		file_label
	);

	FCPPT_RECORD_MAKE_LABEL(
		debug_label
	);

	auto const parser{
		fcppt::options::apply(
			fcppt::options::argument<
				file_label,
				std::filesystem::path
			>{
				fcppt::options::long_name{
					FCPPT_TEXT("file")
				},
				fcppt::options::optional_help_text{}
			},
			fcppt::options::switch_<
				debug_label
			>{
				fcppt::options::optional_short_name{},
				fcppt::options::long_name{
					FCPPT_TEXT("debug")
				},
				fcppt::options::optional_help_text{}
			}
		)
	};

	return
		fcppt::either::match(
			fcppt::options::parse(
				parser,
				fcppt::args_from_second(
					argc,
					argv
				)
			),
			[
				&parser
			](
				fcppt::options::error const &_error
			)
			{
				fcppt::io::cerr()
					<<
					_error
					<<
					FCPPT_TEXT("\nUsage:\n")
					<<
					parser.usage()
					<<
					FCPPT_TEXT('\n');

				return
					EXIT_FAILURE;
			},
			[](
				fcppt::options::result_of<
					decltype(
						parser
					)
				> const &_result
			)
			{
				return
					main_program(
						fcppt::record::get<
							file_label
						>(
							_result
						),
						fcppt::record::get<
							debug_label
						>(
							_result
						)
					);
			}
		);
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
