#include <fcppt/config/external_begin.hpp>
//#include <unistd.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/next_prior.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
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
#include <fcppt/strong_typedef_output.hpp>
#include <fcppt/to_std_string.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/preprocessor/disable_gcc_warning.hpp>
#include <fcppt/preprocessor/pop_warning.hpp>
#include <fcppt/preprocessor/push_warning.hpp>
#include <sge/parse/json/parse_file_exn.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/get_exn.hpp>
#include <sge/parse/json/array.hpp>
#include <sge/parse/json/start.hpp>
#include <regex>
#include <fcppt/io/stream_to_string.hpp>

namespace
{
/*
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
*/

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

optional_compile_command_entry
find_compile_command(
	sge::parse::json::array const &_array,
	boost::filesystem::path const &_input_file_path)
{
	for(
		auto const &current_file
		:
		_array.elements
	)
	{
		compile_command_entry const current_entry(
			entry_from_json(
				sge::parse::json::get_exn<sge::parse::json::object const>(
					current_file)));

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

optional_path
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

std::string const
make_include_paths_absolute(
	std::string const &_compile_command,
	boost::filesystem::path const &_working_directory)
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
		it != parts.end();
		++it)
	{
		if(*it == "-I" || *it == "-isystem")
		{
			boost::filesystem::path const current_path(
				*(it+1));

			if(current_path.is_relative())
				*(it+1) =
					boost::filesystem::canonical(
						_working_directory /
						current_path).string<std::string>();
		}
		else if (it->substr(0,2) == "-I")
		{
			boost::filesystem::path const current_path(
				it->substr(2));

			if(current_path.is_relative())
				*it =
					"-I"+
					boost::filesystem::canonical(
						_working_directory /
						current_path).string<std::string>();
		}
	}

	return
		boost::algorithm::join(
			parts,
			std::string(
				" "));
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

std::string const
extract_include_paths(
	std::string const &_command,
	boost::filesystem::path const &_working_directory)
{
	std::regex e("(?:-I ?|-isystem )([^ ]+)");

	std::string result;

	for(
		std::sregex_iterator
			m1(
				_command.begin(),
				_command.end(),
				e),
		m2;
		m1 != m2;
		++m1)
	{
		boost::filesystem::path current_path(
			(*m1)[1]);

		if(current_path.is_relative())
			current_path = _working_directory / current_path;

		result += boost::filesystem::canonical(current_path).string<std::string>()+"\n";
	}

	return result;
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
			"ignore-compiler-return",
			"Ignore the compiler's return status, always succeed")
		(
			"dump-include-paths",
			"Just dump the include paths, do not compile")
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

	// TODO: Change this
	if(!build_dir.has_value())
	{
		std::cerr << "Build directory not found!\n";
		return EXIT_FAILURE;
	}

	boost::filesystem::path const compile_commands_file(
		build_dir.get_unsafe() / compile_commands_file_name());

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

	// TODO: Change this
	if(!optional_compile_command.has_value())
	{
		std::cerr << "Couldn't find compile command entry for the file!\n";
		return EXIT_FAILURE;
	}

	std::string this_compile_command(
		optional_compile_command.get_unsafe().command());

	this_compile_command =
		make_include_paths_absolute(
			this_compile_command,
			optional_compile_command.get_unsafe().directory());

	if(compiled_options.count("dump-include-paths"))
	{
		std::cout
			<<
				extract_include_paths(
					this_compile_command,
					optional_compile_command.get_unsafe().directory());
		return
			EXIT_SUCCESS;
	}

	if(compiled_options.count("syntax-only"))
		make_syntax_only(
			this_compile_command);
	/*
	std::cout
		<< compile_command->command()
		<< "\n"
		<< compile_command->directory().string<std::string>()
		<< "\n";
	*/
	/*
	check_posix_command_result(
		::chdir(
			optional_compile_command->directory().string<std::string>().c_str()),
		"chdir");
	*/

	std::cout << "Executing " << this_compile_command << "\n";

	int command_exit_status =
		std::system(
			this_compile_command.c_str());

	if(compiled_options.count("ignore-compiler-return"))
		return EXIT_SUCCESS;

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
