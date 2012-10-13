#include <fcppt/config/external_begin.hpp>
#include <unistd.h>
#include <boost/next_prior.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <ostream>
#include <cstdlib>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <fcppt/config/external_end.hpp>
#include <fcppt/strong_typedef.hpp>
#include <fcppt/optional.hpp>
#include <fcppt/string.hpp>
#include <fcppt/to_std_string.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/preprocessor/disable_gcc_warning.hpp>
#include <fcppt/preprocessor/pop_warning.hpp>
#include <fcppt/preprocessor/push_warning.hpp>
#include <sge/parse/json/parse_file_exn.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/get.hpp>
#include <sge/parse/json/array.hpp>
#include <sge/parse/json/start.hpp>
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
	while(!boost::filesystem::is_directory(_path / _build_directory.get()))
	{
		if(_path.empty())
			return optional_path();

		_path = _path.parent_path();
	}

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

sge::parse::json::array const
parse_compile_commands_file(
	boost::filesystem::path const &_path)
{
	if(
		!boost::filesystem::exists(
			_path))
		throw
			std::runtime_error(
				_path.string()
				+
				" does not exist!"
			);

	return
		sge::parse::json::parse_file_exn(
			_path
		).array();
}

void
make_syntax_only(
	std::string &_compile_command)
{
	typedef
	std::vector<std::string>
	string_sequence;

	string_sequence parts;

	boost::algorithm::split(
		parts,
		_compile_command,
		boost::algorithm::is_space(),
		boost::algorithm::token_compress_on);

	for(
		string_sequence::iterator it =
			parts.begin();
		it != parts.end();)
	{
		if(*it == "-o")
		{
			it =
				parts.erase(
					it,
					it+2);
		}
		else
		{
			++it;
		}
	}

	parts.push_back("-fsyntax-only");

	_compile_command =
		boost::algorithm::join(
			parts,
			std::string(
				" "));
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
			"syntax-only",
			"Do not output anything, just check the syntax")
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

	sge::parse::json::array const contents(
		parse_compile_commands_file(
			compile_commands_file));

	optional_compile_command_entry const optional_compile_command(
		find_compile_command(
			contents,
			input_file_path));

	if(!optional_compile_command)
	{
		std::cerr << "Couldn't find compile command entry for the file!\n";
		return EXIT_FAILURE;
	}

	std::string compile_command =
		optional_compile_command->command();

	if(compiled_options.count("syntax-only"))
		make_syntax_only(
			compile_command);
	/*
	std::cout
		<< compile_command->command()
		<< "\n"
		<< compile_command->directory().string<std::string>()
		<< "\n";
	*/
	check_posix_command_result(
		::chdir(
			optional_compile_command->directory().string<std::string>().c_str()),
		"chdir");

	int command_exit_status =
		std::system(
			compile_command.c_str());

FCPPT_PP_PUSH_WARNING
FCPPT_PP_DISABLE_GCC_WARNING(-Wold-style-cast)
	return
		WEXITSTATUS(command_exit_status);
FCPPT_PP_POP_WARNING
}
catch(std::exception const &e)
{
	std::cerr << "error: " << e.what() << "\n";
	return EXIT_FAILURE;
}
