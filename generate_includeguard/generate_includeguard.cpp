#include <fcppt/args_char.hpp>
#include <fcppt/args_from_second.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/main.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <fcppt/either/match.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/options/argument.hpp>
#include <fcppt/options/error.hpp>
#include <fcppt/options/error_output.hpp>
#include <fcppt/options/long_name.hpp>
#include <fcppt/options/optional_help_text.hpp>
#include <fcppt/options/parse.hpp>
#include <fcppt/options/result_of.hpp>
#include <fcppt/options/usage_output.hpp>
#include <fcppt/record/get.hpp>
#include <fcppt/record/make_label.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <exception>
#include <filesystem>
#include <iostream>
#include <list>
#include <utility>
#include <fcppt/config/external_end.hpp>

namespace
{
using directory_sequence = std::list<fcppt::string>;

/*
 * Example: We're in project/include/foo/bar/baz.hpp and call the program
 *
 * project/include/foo/bar/ Isn't include/ => recurse
 *   project/include/foo/ Isn't include/ => recurse
 *     project/include/ Is include, so return nothing
 *   got {}, adding foo, returning {foo}
 * got {foo}, adding bar, returning {foo,bar}
 */
directory_sequence calculate_sequence(std::filesystem::path const &current)
{
  if (fcppt::filesystem::path_to_string(current.filename()) == FCPPT_TEXT("include"))
  {
    return directory_sequence();
  }

  if (!current.has_parent_path())
  {
    throw fcppt::exception(FCPPT_TEXT("Arrived at the root, didn't find include/"));
  }

  directory_sequence result = calculate_sequence(current.parent_path());

  result.push_back(fcppt::filesystem::path_to_string(current.filename()));

  return result;
}

fcppt::string create_guard(directory_sequence const &dirs, fcppt::string const &fn)
{
  fcppt::string guard;
  for (auto i = dirs.begin(); i != dirs.end(); ++i)
  {
    if (i != dirs.begin())
    {
      guard += FCPPT_TEXT('_');
    }

    guard += boost::algorithm::to_upper_copy( // NOLINT(fuchsia-default-arguments-calls)
        *i);
  }

  guard += FCPPT_TEXT('_') +
           boost::algorithm::to_upper_copy( // NOLINT(fuchsia-default-arguments-calls)
               boost::algorithm::replace_all_copy(fn, FCPPT_TEXT("."), FCPPT_TEXT("_")));

  return guard + FCPPT_TEXT("_INCLUDED");
}

std::pair<fcppt::string, fcppt::string> create_namespaces(directory_sequence const &dirs)
{
  std::pair<fcppt::string, fcppt::string> output;
  for (auto const &d : dirs)
  {
    output.first += FCPPT_TEXT("namespace ") + d + FCPPT_TEXT("\n{\n");
    output.second += FCPPT_TEXT("}\n");
  }
  return output;
}

void main_program(std::filesystem::path const &_p)
{
  // We might open a new buffer in vim to which no (saved) file is attached yet, so this test is useless
  //if (!std::filesystem::is_regular_file(p))
  //{
  //	fcppt::io::cerr() << FCPPT_TEXT("The file ") << p << FCPPT_TEXT(" is not regular\n");
  //	return EXIT_FAILURE;
  //}

  directory_sequence const s = calculate_sequence(_p.parent_path());

  std::pair<fcppt::string, fcppt::string> const namespaces = create_namespaces(s);

  fcppt::string const guard = create_guard(s, fcppt::filesystem::path_to_string(_p.filename()));

  fcppt::io::cout() << FCPPT_TEXT("#ifndef ") << guard << FCPPT_TEXT("\n") << FCPPT_TEXT("#define ")
                    << guard << FCPPT_TEXT("\n\n") << namespaces.first << namespaces.second
                    << FCPPT_TEXT("\n") << FCPPT_TEXT("#endif");
}

}

int FCPPT_MAIN(int argc, fcppt::args_char **argv)
try
{
  FCPPT_RECORD_MAKE_LABEL(file_label);

  fcppt::options::argument<file_label, std::filesystem::path> const parser{
      fcppt::options::long_name{FCPPT_TEXT("filename")}, fcppt::options::optional_help_text{}};

  return fcppt::either::match(
      fcppt::options::parse(parser, fcppt::args_from_second(argc, argv)),
      [&parser](fcppt::options::error const &_error)
      {
        fcppt::io::cerr() << _error << FCPPT_TEXT("\nUsage:\n") << parser.usage()
                          << FCPPT_TEXT('\n');

        return EXIT_FAILURE;
      },
      [](fcppt::options::result_of<decltype(parser)> const &_result)
      {
        main_program(fcppt::record::get<file_label>(_result));

        return EXIT_SUCCESS;
      });
}
catch (fcppt::exception const &e)
{
  fcppt::io::cerr() << e.string() << FCPPT_TEXT("\n");
  return EXIT_FAILURE;
}
catch (std::exception const &e)
{
  std::cerr << e.what() << '\n';
  return EXIT_FAILURE;
}
