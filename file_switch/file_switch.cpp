#include <fcppt/assert/error_message.hpp>
#include <fcppt/text.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <algorithm>
#include <exception>
#include <fstream>
#include <numeric>
#include <iterator>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <boost/next_prior.hpp>
#include <fcppt/config/external_end.hpp>

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
main(
	int argc,
	char **argv
)
try
{
	if(
		argc <= 1)
	{
		std::cerr
			<< "Usage: "
			<< argv[0]
			<< " <hpp-or-cpp>\n";

		return EXIT_FAILURE;
	}

	boost::filesystem::path const
		input_file_with_extension(
			argv[1]);

	boost::filesystem::path
		input_directory(
			input_file_with_extension.parent_path()),
		namespace_path;

	bool const debug_mode =
		false;

	if(debug_mode)
	{
		std::cout << "input_file_with_extension: " << input_file_with_extension << "\n";
		std::cout << "input_directory: " << input_directory << "\n";
	}

	while(
		!boost::filesystem::is_directory(input_directory / "src") ||
		!boost::filesystem::is_directory(input_directory / "include"))
	{
		namespace_path = *(boost::prior(input_directory.end())) / namespace_path;
		input_directory = input_directory.parent_path();

		if(input_directory.empty())
		{
			std::cerr << "error: no directory found where include/ and src/ lie below. Exiting...\n";
			return EXIT_FAILURE;
		}

		if(debug_mode)
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
	bool const is_header = namespace_path.begin()->string<std::string>() == "include";

	if(is_header && input_file_with_extension.extension().string<std::string>() != ".hpp")
	{
		std::cerr << "Got a file in include/ which doesn't end in \"hpp\" (extension is " << input_file_with_extension.extension().string<std::string>() << ").\n";
		std::cerr << "Not sure what to do, exiting...\n";
		return EXIT_FAILURE;
	}

	if(!is_header && input_file_with_extension.extension().string<std::string>() != ".cpp")
	{
		std::cerr << "Got a file in src/ which doesn't end in \"cpp\". Not sure what to do, exiting...\n";
		return EXIT_FAILURE;
	}

	FCPPT_ASSERT_ERROR_MESSAGE(
		!is_header || std::distance(namespace_path.begin(),namespace_path.end()) > 1,
		FCPPT_TEXT("There's no directory below include/ denoting the namespace of the project. Don't know how to handle that yet."));

	boost::filesystem::path const trimmed_namespace_path =
		std::accumulate(
			boost::next(
				namespace_path.begin(),
				is_header
				?
					2
				:
					1),
			namespace_path.end(),
			boost::filesystem::path(),
			std::divides<boost::filesystem::path>());

	if(debug_mode)
	{
		std::cout << "Trimmed namespace path: " << trimmed_namespace_path << "\n";
	}

	boost::filesystem::directory_iterator include_directory_iterator(
		input_directory / "include");

	if(include_directory_iterator == boost::filesystem::directory_iterator())
	{
		std::cerr << "The include directory " << (input_directory / "include") << " is empty. Exiting...\n";
		return EXIT_FAILURE;
	}

	std::string const namespace_name(
		include_directory_iterator->path().stem().string<std::string>());

	if(debug_mode)
	{
		std::cout << "Namespace name: " << namespace_name << "\n";
	}

	if(is_header)
	{
		std::cout << (input_directory / "src" / trimmed_namespace_path / input_file_with_extension.stem().replace_extension(".cpp")).string<std::string>();
	}
	else
	{
		std::cout << (input_directory / "include" / namespace_name / trimmed_namespace_path / input_file_with_extension.stem().replace_extension(".hpp")).string<std::string>();
	}

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
