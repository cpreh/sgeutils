#include <fcppt/args_char.hpp>
#include <fcppt/args_from_second.hpp>
#include <fcppt/error_code_to_string.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/main.hpp>
#include <fcppt/string.hpp>
#include <fcppt/system.hpp>
#include <fcppt/text.hpp>
#include <fcppt/algorithm/fold.hpp>
#include <fcppt/either/match.hpp>
#include <fcppt/filesystem/extension_without_dot.hpp>
#include <fcppt/filesystem/make_recursive_directory_range.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/filesystem/recursive_directory_range.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/optional/maybe.hpp>
#include <fcppt/options/apply.hpp>
#include <fcppt/options/argument.hpp>
#include <fcppt/options/error.hpp>
#include <fcppt/options/error_output.hpp>
#include <fcppt/options/long_name.hpp>
#include <fcppt/options/make_many.hpp>
#include <fcppt/options/no_default_value.hpp>
#include <fcppt/options/option.hpp>
#include <fcppt/options/optional_help_text.hpp>
#include <fcppt/options/optional_short_name.hpp>
#include <fcppt/options/parse.hpp>
#include <fcppt/options/result_of.hpp>
#include <fcppt/options/short_name.hpp>
#include <fcppt/options/switch.hpp>
#include <fcppt/record/element.hpp>
#include <fcppt/record/get.hpp>
#include <fcppt/record/make_label.hpp>
#include <fcppt/record/object_impl.hpp>
#include <fcppt/config/external_begin.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <utility>
#include <vector>
#include <fcppt/config/external_end.hpp>

namespace
{

FCPPT_RECORD_MAKE_LABEL(pretend_label);

FCPPT_RECORD_MAKE_LABEL(library_label);

FCPPT_RECORD_MAKE_LABEL(path_label);

using args_record = fcppt::record::object<
    fcppt::record::element<pretend_label, bool>,
    fcppt::record::element<library_label, std::vector<fcppt::string>>,
    fcppt::record::element<path_label, std::vector<std::filesystem::path>>>;

int run_command(fcppt::string const &_command, int const _prev_result)
try
{
  int result{_prev_result};

  fcppt::optional::maybe(
      fcppt::system(_command),
      [&result, &_command]
      {
        fcppt::io::cerr() << FCPPT_TEXT("Failed to execute command (no result) ") << _command
                          << FCPPT_TEXT('\n');

        result = EXIT_FAILURE;
      },
      [&result, &_command](int const _code)
      {
        if (_code != EXIT_SUCCESS)
        {
          fcppt::io::cerr() << FCPPT_TEXT("Failed to execute command ") << _command
                            << FCPPT_TEXT('\n');

          result = EXIT_FAILURE;
        }
      });

  return result;
}
catch (fcppt::exception const &_error)
{
  fcppt::io::cerr() << _error.string() << FCPPT_TEXT('\n');

  return EXIT_FAILURE;
}

int main_program(args_record const &_args)
{
  int result{EXIT_SUCCESS};

  fcppt::string const command_line{fcppt::algorithm::fold(
      fcppt::record::get<library_label>(_args),
      fcppt::string{},
      [](fcppt::string const &_value, fcppt::string &&_state)
      { return std::move(_state) + FCPPT_TEXT(" --library ") + _value + FCPPT_TEXT(' '); })};

  for (std::filesystem::path const &path : fcppt::record::get<path_label>(_args))
  {
    fcppt::either::match(
        fcppt::filesystem::make_recursive_directory_range(
            path, std::filesystem::directory_options::none),
        [&result, &path](std::error_code const _error)
        {
          fcppt::io::cerr() << FCPPT_TEXT("Failed to open \"")
                            << fcppt::filesystem::path_to_string(path) << FCPPT_TEXT("\": ")
                            << fcppt::error_code_to_string(_error) << FCPPT_TEXT('\n');

          result = EXIT_FAILURE;
        },
        [&_args, &command_line, &result](fcppt::filesystem::recursive_directory_range const &_range)
        {
          for (std::filesystem::path const &inner : _range)
          {
            fcppt::string const extension{fcppt::filesystem::extension_without_dot(inner)};

            if (extension == FCPPT_TEXT("cpp") || extension == FCPPT_TEXT("hpp"))
            {
              fcppt::string const command{
                  FCPPT_TEXT("update_headers.sh ") + fcppt::filesystem::path_to_string(inner) +
                  command_line};

              if (fcppt::record::get<pretend_label>(_args))
              {
                fcppt::io::cout() << command << FCPPT_TEXT('\n');
              }
              else
              {
                result = run_command(command, result);
              }
            }
          }
        });
  }

  return result;
}

}

int FCPPT_MAIN(int argc, fcppt::args_char **argv)
try
{
  auto const parser{fcppt::options::apply(
      fcppt::options::switch_<pretend_label>{
          fcppt::options::optional_short_name{fcppt::options::short_name{FCPPT_TEXT("p")}},
          fcppt::options::long_name{FCPPT_TEXT("pretend")},
          fcppt::options::optional_help_text{}},
      fcppt::options::make_many(fcppt::options::option<library_label, fcppt::string>{
          fcppt::options::optional_short_name{},
          fcppt::options::long_name{FCPPT_TEXT("library")},
          fcppt::options::no_default_value<fcppt::string>(),
          fcppt::options::optional_help_text{}}),
      fcppt::options::make_many(fcppt::options::argument<path_label, std::filesystem::path>{
          fcppt::options::long_name{FCPPT_TEXT("path")}, fcppt::options::optional_help_text{}}))};

  return fcppt::either::match(
      fcppt::options::parse(parser, fcppt::args_from_second(argc, argv)),
      [&parser](fcppt::options::error const &_error)
      {
        fcppt::io::cerr() << _error << FCPPT_TEXT("\nUsage: ") << parser.usage()
                          << FCPPT_TEXT('\n');

        return EXIT_FAILURE;
      },
      [](fcppt::options::result_of<decltype(parser)> &&_result)
      { return main_program(args_record{std::move(_result)}); });
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
