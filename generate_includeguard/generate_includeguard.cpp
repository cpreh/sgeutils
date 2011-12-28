#include <fcppt/exception.hpp>
#include <fcppt/filesystem/is_directory.hpp>
#include <fcppt/filesystem/path.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <list>
#include <iterator>
#include <streambuf>
#include <fcppt/config/external_end.hpp>

namespace
{
typedef
std::list<fcppt::string>
directory_sequence;

/*
 * Example: We're in project/include/foo/bar/baz.hpp and call the program
 *
 * project/include/foo/bar/ Isn't include/ => recurse
 *   project/include/foo/ Isn't include/ => recurse
 *     project/include/ Is include, so return nothing
 *   got {}, adding foo, returning {foo}
 * got {foo}, adding bar, returning {foo,bar}
 */
directory_sequence const
calculate_sequence(
	fcppt::filesystem::path const &current)
{
	if(
		fcppt::filesystem::path_to_string(
			current.filename())
		== FCPPT_TEXT("include")
	)
		return directory_sequence();

	if (!current.has_parent_path())
		throw fcppt::exception(FCPPT_TEXT("Arrived at the root, didn't find include/"));

	directory_sequence result =
		calculate_sequence(
			current.parent_path());

	result.push_back(
		fcppt::filesystem::path_to_string(
			current.filename()));

	return result;
}

fcppt::string const
create_guard(
	directory_sequence const &dirs,
	fcppt::string const &fn)
{
	fcppt::string guard;
	for (directory_sequence::const_iterator i = dirs.begin(); i != dirs.end(); ++i)
	{
		if (i != dirs.begin())
			guard +=
				FCPPT_TEXT('_');

		guard +=
			boost::algorithm::to_upper_copy(
				*i);
	}

	guard +=
		FCPPT_TEXT('_') +
		boost::algorithm::to_upper_copy(
			boost::algorithm::replace_all_copy(
				fn,
				FCPPT_TEXT("."),
				FCPPT_TEXT("_")));

	return guard + FCPPT_TEXT("_INCLUDED");
}

std::pair<fcppt::string,fcppt::string> const
create_namespaces(
	directory_sequence const &dirs)
{
	std::pair<fcppt::string,fcppt::string> output;
	for (directory_sequence::const_iterator i = dirs.begin(); i != dirs.end(); ++i)
	{
		output.first += FCPPT_TEXT("namespace ")+(*i)+FCPPT_TEXT("\n{\n");
		output.second += FCPPT_TEXT("}\n");
	}
	return output;
}
}

int main(
	int const argc,
	char **argv)
try
{
	if (argc != 2)
	{
		fcppt::io::cerr() << FCPPT_TEXT("usage: ") << fcppt::from_std_string(argv[0]) << FCPPT_TEXT(" <filename>\n");
		return EXIT_FAILURE;
	}

	fcppt::filesystem::path const p(
		fcppt::from_std_string(
			argv[1]));

	// We might open a new buffer in vim to which no (saved) file is attached yet, so this test is useless
	//if (!fcppt::filesystem::is_regular(p))
	//{
	//	fcppt::io::cerr() << FCPPT_TEXT("The file ") << p << FCPPT_TEXT(" is not regular\n");
	//	return EXIT_FAILURE;
	//}

	directory_sequence const s =
		calculate_sequence(
			p.parent_path());

	std::pair<fcppt::string,fcppt::string> const namespaces =
		create_namespaces(
			s);

	fcppt::string const guard =
		create_guard(
			s,
			fcppt::filesystem::path_to_string(
				p.filename()));

	fcppt::io::cout()
		<< FCPPT_TEXT("#ifndef ") << guard << FCPPT_TEXT("\n")
		<< FCPPT_TEXT("#define ") << guard << FCPPT_TEXT("\n\n")
		<< namespaces.first
		<< namespaces.second
		<< FCPPT_TEXT("\n")
		<< FCPPT_TEXT("#endif");
}
catch (fcppt::exception const &e)
{
	fcppt::io::cerr() << e.string() << FCPPT_TEXT("\n");
	return EXIT_FAILURE;
}
