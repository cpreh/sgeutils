#include <fcppt/config/external_begin.hpp>
#include <unistd.h>
#include <iostream>
#include <ostream>
#include <cstdlib>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <fcppt/config/external_end.hpp>
#include <fcppt/strong_typedef.hpp>
#include <fcppt/optional.hpp>
#include <fcppt/string.hpp>
#include <fcppt/assert/pre.hpp>
#include <fcppt/to_std_string.hpp>
#include <sge/parse/json/parse_string_exn.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/get.hpp>
#include <sge/parse/json/array.hpp>
#include <fcppt/io/stream_to_string.hpp>

namespace
{
void
check_posix_command_result(
	int const result,
	std::string const &_name)
{
	if(result)
		throw
			std::runtime_error(
				_name+
				" failed: "+
				std::strerror(
					errno));
}

class compile_command_entry
{
public:
	compile_command_entry(
		boost::filesystem::path const &_directory,
		std::string const &_command,
		boost::filesystem::path const &_file)
	:
		directory_(
			_directory),
		command_(
			_command),
		file_(
			_file)
	{
	}

	boost::filesystem::path const &
	directory() const
	{
		return
			directory_;
	}

	std::string const &
	command() const
	{
		return
			command_;
	}

	boost::filesystem::path const &
	file() const
	{
		return
			file_;
	}
private:
	boost::filesystem::path directory_;
	std::string command_;
	boost::filesystem::path file_;
};

compile_command_entry const
entry_from_json(
	sge::parse::json::object const &_json)
{
	return
		compile_command_entry(
			boost::filesystem::path(
				fcppt::string(
					sge::parse::json::find_member_exn<sge::parse::json::string const>(
						_json.members,
						fcppt::string(
							FCPPT_TEXT("directory"))))),
			fcppt::to_std_string(
				sge::parse::json::find_member_exn<sge::parse::json::string const>(
					_json.members,
					fcppt::string(
						FCPPT_TEXT("command")))),
			boost::filesystem::path(
				fcppt::string(
					sge::parse::json::find_member_exn<sge::parse::json::string const>(
						_json.members,
						fcppt::string(
							FCPPT_TEXT("file"))))));
}


typedef
fcppt::optional<compile_command_entry>
optional_compile_command_entry;

optional_compile_command_entry const
find_compile_command(
	sge::parse::json::array const &_array,
	boost::filesystem::path const &_input_file_path)
{
	for(
		sge::parse::json::element_vector::const_iterator current_file =
			_array.elements.begin();
		current_file != _array.elements.end();
		++current_file)
	{
		compile_command_entry const current_entry(
			entry_from_json(
				sge::parse::json::get<sge::parse::json::object const>(
					*current_file)));

		if(current_entry.file() == _input_file_path)
			return
				optional_compile_command_entry(
					current_entry);
	}

	return
		optional_compile_command_entry();
}

FCPPT_MAKE_STRONG_TYPEDEF(
	std::string,
	build_directory_name);

typedef
fcppt::optional<boost::filesystem::path>
optional_path;

optional_path const
find_build_directory(
	boost::filesystem::path _path,
	build_directory_name const &_build_directory)
{
	if(_path.empty())
		return optional_path();

	while(!boost::filesystem::is_directory(_path / _build_directory.get()))
		_path = _path.parent_path();

	return
		optional_path(
			_path / _build_directory.get());
}

std::string const
compile_commands_file_name()
{
	return
		std::string("compile_commands.json");
}

sge::parse::json::object const
parse_compile_commands_file(
	boost::filesystem::path const &_path)
{
	boost::filesystem::ifstream file_stream(
		_path);

	FCPPT_ASSERT_PRE(
		file_stream.is_open());

	return
		sge::parse::json::parse_string_exn(
			"{ \"content\" : "+
			fcppt::io::stream_to_string(
				file_stream)+
			" }");
}
}

int
main(
	int argc,
	char **argv)
try
{
	std::string input_file_name,build_dir_name;

	boost::program_options::options_description allowed_options_description("Allowed options");
	allowed_options_description.add_options()
		(
			"help",
			"produce help message")
		(
			"file-name",
			boost::program_options::value<std::string>(&input_file_name)->required(),
			"Input file name")
		(
			"build-dir-name",
			boost::program_options::value<std::string>(&build_dir_name)->default_value(std::string("sgebuild_gcc")),
			"Name of the directory containing the compile_commands.json file (note: this is not an absolute path, just the last part of one)");

	boost::program_options::variables_map compiled_options;
	boost::program_options::store(
		boost::program_options::parse_command_line(
			argc,
			argv,
			allowed_options_description),
		compiled_options);

	if(compiled_options.count("help"))
	{
		std::cout << allowed_options_description << "\n";
		return
			EXIT_SUCCESS;
	}

	boost::program_options::notify(
		compiled_options);

	boost::filesystem::path const input_file_path(
		input_file_name);

	optional_path const build_dir(
		find_build_directory(
			input_file_path,
			build_directory_name(
				build_dir_name)));

	if(!build_dir)
	{
		std::cerr << "Build directory not found!\n";
		return EXIT_FAILURE;
	}

	boost::filesystem::path const compile_commands_file(
		*(build_dir) / compile_commands_file_name());

	if(!boost::filesystem::exists(compile_commands_file))
	{
		std::cerr << "Build directory found, but couldn't locate " << compile_commands_file_name() << "!\n";
		return EXIT_FAILURE;
	}

	sge::parse::json::object const json_content(
		parse_compile_commands_file(
			compile_commands_file));

	sge::parse::json::array const &contents(
		sge::parse::json::find_member_exn<sge::parse::json::array const>(
			json_content.members,
			fcppt::string(
				FCPPT_TEXT("content"))));

	optional_compile_command_entry const compile_command(
		find_compile_command(
			contents,
			input_file_path));

	if(!compile_command)
	{
		std::cerr << "Couldn't find compile command entry for the file!\n";
		return EXIT_FAILURE;
	}

	/*
	std::cout
		<< compile_command->command()
		<< "\n"
		<< compile_command->directory().string<std::string>()
		<< "\n";
	*/
	check_posix_command_result(
		::chdir(
			compile_command->directory().string<std::string>().c_str()),
		"chdir");

	return
		std::system(
			compile_command->command().c_str());
}
catch(std::exception const &e)
{
	std::cerr << "error: " << e.what() << "\n";
	return EXIT_FAILURE;
}
