#include <fcppt/args_char.hpp>
#include <fcppt/args_from_second.hpp>
#include <fcppt/char_type.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/main.hpp>
#include <fcppt/string.hpp>
#include <fcppt/text.hpp>
#include <fcppt/either/match.hpp>
#include <fcppt/filesystem/extension_without_dot.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/filesystem/remove_extension.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/optional/from.hpp>
#include <fcppt/options/apply.hpp>
#include <fcppt/options/argument.hpp>
#include <fcppt/options/error.hpp>
#include <fcppt/options/error_output.hpp>
#include <fcppt/options/long_name.hpp>
#include <fcppt/options/make_optional.hpp>
#include <fcppt/options/option.hpp>
#include <fcppt/options/optional_help_text.hpp>
#include <fcppt/options/parse.hpp>
#include <fcppt/options/result_of.hpp>
#include <fcppt/options/usage_output.hpp>
#include <fcppt/record/get.hpp>
#include <fcppt/record/make_label.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <fcppt/config/external_end.hpp>

namespace
{

std::filesystem::path::iterator::difference_type
path_levels(std::filesystem::path const &_base_path)
{
  std::filesystem::path::iterator::difference_type ret(
      std::distance(_base_path.begin(), _base_path.end()));

  if (ret > 0 &&
      fcppt::filesystem::path_to_string(*std::prev( // NOLINT(fuchsia-default-arguments-calls)
                                            _base_path.end()))
          .empty())
  {
    --ret;
  }

  return ret;
}

fcppt::string make_include_guard(
    std::filesystem::path::iterator::difference_type const _base_level,
    std::filesystem::path const &_path)
{
  fcppt::string ret;

  std::filesystem::path const without_extension(fcppt::filesystem::remove_extension(_path));

  if(std::distance(_path.begin(), _path.end()) < _base_level)
  {
     throw fcppt::exception{FCPPT_TEXT("Path too short")};
  }

  for (std::filesystem::path const &path : boost::make_iterator_range(
           std::next(without_extension.begin(), _base_level), without_extension.end()))
  {
    ret += boost::algorithm::to_upper_copy( // NOLINT(fuchsia-default-arguments-calls)
               fcppt::filesystem::path_to_string(path)) +
           FCPPT_TEXT('_');
  }

  return ret +
         boost::algorithm::to_upper_copy( // NOLINT(fuchsia-default-arguments-calls)
             fcppt::filesystem::extension_without_dot(_path)) +
         FCPPT_TEXT("_INCLUDED");
}

bool is_reserved_identifier(fcppt::string const &_identifier)
{
  return !_identifier.empty() && (_identifier[0] == FCPPT_TEXT('_') ||
                                  _identifier.find(FCPPT_TEXT("__")) != fcppt::string::npos);
}

void main_program(std::filesystem::path const &_base_path, fcppt::string const &_prefix)
{
  std::filesystem::path::iterator::difference_type const base_levels(::path_levels(_base_path));

  for (std::filesystem::path const &path : boost::make_iterator_range(
           // NOLINTNEXTLINE(fuchsia-default-arguments-calls)
           std::filesystem::recursive_directory_iterator(_base_path),
           std::filesystem::recursive_directory_iterator()))
  {
    if (!std::filesystem::is_regular_file(path))
    {
      continue;
    }

    fcppt::string const extension(fcppt::filesystem::extension_without_dot(path));

    if (extension != FCPPT_TEXT("hpp") && extension != FCPPT_TEXT("h"))
    {
      continue;
    }

    std::basic_ifstream<fcppt::char_type> stream( // NOLINT(fuchsia-default-arguments-calls)
        path);

    if (!stream.is_open())
    {
      fcppt::io::cerr() << FCPPT_TEXT("Failed to open ") << path << FCPPT_TEXT('\n');

      continue;
    }

    fcppt::string const include_guard(_prefix + ::make_include_guard(base_levels, path));

    fcppt::string const ifndef_line(FCPPT_TEXT("#ifndef ") + include_guard);

    fcppt::string const define_line(FCPPT_TEXT("#define ") + include_guard);

    fcppt::string line;

    bool guard_found = false;

    while (std::getline(stream, line))
    {
      fcppt::string line2;

      if (line == ifndef_line && (std::getline(stream, line2) && line2 == define_line))
      {
        guard_found = true;

        break;
      }
    }

    if (is_reserved_identifier(include_guard))
    {
      fcppt::io::cout() << path << FCPPT_TEXT(": Include guard ") << include_guard
                        << FCPPT_TEXT(" is a reserved identifier.\n");
    }

    if (!guard_found)
    {
      fcppt::io::cout() << path << FCPPT_TEXT(": ") << include_guard << FCPPT_TEXT('\n');
    }
  }
}

}

int FCPPT_MAIN(int argc, fcppt::args_char **argv)
try
{
  FCPPT_RECORD_MAKE_LABEL(path_label);

  FCPPT_RECORD_MAKE_LABEL(prefix_label);

  auto const parser{fcppt::options::apply(
      fcppt::options::argument<path_label, std::filesystem::path>{
          fcppt::options::long_name{FCPPT_TEXT("path")}, fcppt::options::optional_help_text{}},
      fcppt::options::make_optional(fcppt::options::argument<prefix_label, fcppt::string>{
          fcppt::options::long_name{FCPPT_TEXT("prefix")}, fcppt::options::optional_help_text{}}))};

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
        main_program(
            std::filesystem::canonical(fcppt::record::get<path_label>(_result)),
            fcppt::optional::from(
                fcppt::record::get<prefix_label>(_result), [] { return fcppt::string{}; }));

        return EXIT_SUCCESS;
      });
}
catch (fcppt::exception const &_error)
{
  fcppt::io::cerr() << _error.string() << FCPPT_TEXT('\n');

  return EXIT_FAILURE;
}
catch (std::exception const &_error)
{
  std::cerr << _error.what() << '\n';

  return EXIT_FAILURE;
}
