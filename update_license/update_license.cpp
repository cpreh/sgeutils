#include <fcppt/args_char.hpp>
#include <fcppt/args_from_second.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/main.hpp>
#include <fcppt/make_cref.hpp>
#include <fcppt/text.hpp>
#include <fcppt/either/match.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/options/apply.hpp>
#include <fcppt/options/argument.hpp>
#include <fcppt/options/error.hpp>
#include <fcppt/options/error_output.hpp>
#include <fcppt/options/long_name.hpp>
#include <fcppt/options/optional_help_text.hpp>
#include <fcppt/options/parse.hpp>
#include <fcppt/options/result_of.hpp>
#include <fcppt/options/usage_output.hpp>
#include <fcppt/parse/char.hpp>
#include <fcppt/parse/make_literal.hpp>
#include <fcppt/parse/parse_stream.hpp>
#include <fcppt/parse/string.hpp>
#include <fcppt/parse/operators/alternative.hpp>
#include <fcppt/parse/operators/exclude.hpp>
#include <fcppt/parse/operators/repetition.hpp>
#include <fcppt/parse/operators/sequence.hpp>
#include <fcppt/record/get.hpp>
#include <fcppt/record/make_label.hpp>
#include <fcppt/config/external_begin.hpp>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <ostream>
#include <string>
#include <fcppt/config/external_end.hpp>

namespace
{

int main_program(std::filesystem::path const &_file, std::filesystem::path const &_license_file)
{
  std::ifstream ifs{_file};

  if (!ifs.is_open())
  {
    std::cerr << "Cannot open " << _file.string() << '\n';

    return EXIT_FAILURE;
  }

  {
    ifs >> std::noskipws;

    auto const eol{fcppt::parse::make_literal('\n')};

    auto const ml_comment{
        fcppt::parse::string{"/*"} >> *(fcppt::parse::char_{} - fcppt::parse::string{"*/"}) >>
        fcppt::parse::string{"*/"}};

    auto const comment{
        fcppt::parse::string{"//"} >> *(fcppt::parse::char_{} - fcppt::make_cref(eol)) >>
        fcppt::make_cref(eol)};

    auto const parser{
        *(fcppt::make_cref(ml_comment) | fcppt::make_cref(comment)) >> *fcppt::make_cref(eol)};

    if(fcppt::parse::parse_stream(parser, ifs).has_failure())
    {
      std::cerr << "Parsing license header failed.\n";
      return EXIT_FAILURE;
    }
  }

  ifs.unget(); // FIXME

  if (!ifs.good())
  {
    std::cerr << "Input file " << _file.string() << " broken\n";

    return EXIT_FAILURE;
  }

  std::ifstream const license{_license_file};

  if (!license.is_open())
  {
    std::cerr << "Cannot open license file " << _license_file.string() << '\n';

    return EXIT_FAILURE;
  }

  std::filesystem::path const temp_filename{_file.string() + std::string{".temp"}};

  {
    std::ofstream ofs{temp_filename};

    if (!ofs.is_open())
    {
      std::cerr << "Cannot open " << temp_filename << '\n';

      return EXIT_FAILURE;
    }

    ofs << license.rdbuf() << ifs.rdbuf();
  }

  {
    ifs.clear(); // NOLINT(fuchsia-default-arguments-calls)

    ifs.seekg(std::ios_base::beg);

    if (!ifs)
    {
      std::cerr << "Rewinding " << _file.string() << " failed\n";

      return EXIT_FAILURE;
    }

    std::ifstream new_ifs{// NOLINT(fuchsia-default-arguments-calls)
                          temp_filename};

    if (!new_ifs)
    {
      std::cerr << "Opening " << temp_filename << " failed\n";

      return EXIT_FAILURE;
    }

    std::string line1{};
    std::string line2{};

    bool equal(true);

    while (std::getline(ifs, line1) && std::getline(new_ifs, line2))
    {
      if (line1 != line2)
      {
        equal = false;

        break;
      }
    }

    if (equal)
    {
      new_ifs.close();

      if (std::remove(temp_filename.string().c_str()) != 0)
      {
        std::cerr << "Cannot remove " << temp_filename << '\n';

        return EXIT_FAILURE;
      }

      return EXIT_SUCCESS;
    }
  }

  ifs.close();

  if (std::rename(temp_filename.string().c_str(), _file.string().c_str()) != 0)
  {
    std::cerr << "Cannot rename " << temp_filename << " into " << _file.string() << '\n';

    if (std::remove(temp_filename.string().c_str()) != 0)
    {
      std::cerr << "Cannot remove " << temp_filename << "! You have to remove it manually!\n";
    }

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

}

int FCPPT_MAIN(int argc, fcppt::args_char **argv)
try
{
  FCPPT_RECORD_MAKE_LABEL(file_label);

  FCPPT_RECORD_MAKE_LABEL(license_file_label);

  auto const parser{fcppt::options::apply(
      fcppt::options::argument<file_label, std::filesystem::path>{
          fcppt::options::long_name{FCPPT_TEXT("file")}, fcppt::options::optional_help_text{}},
      fcppt::options::argument<license_file_label, std::filesystem::path>{
          fcppt::options::long_name{FCPPT_TEXT("license-file")},
          fcppt::options::optional_help_text{}})};

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
        return main_program(
            fcppt::record::get<file_label>(_result),
            fcppt::record::get<license_file_label>(_result));
      });
}
catch (fcppt::exception const &_e)
{
  fcppt::io::cerr() << FCPPT_TEXT("Caught an exception: ") << _e.string() << FCPPT_TEXT("\n");

  return EXIT_FAILURE;
}
catch (std::exception const &_error)
{
  std::cerr << _error.what() << '\n';

  return EXIT_FAILURE;
}
