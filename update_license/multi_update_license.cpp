/*
Example file:

{
  "main_license" : "foo.license"
  "exceptions" :
  [
    {
      "files" : ["foo/bar/.*","baz"]
      "license" : ["bar.license"]
    },
    ...
  ]
  "nolicense" : ["baz/qux/.*",...]
}

Assertions:

-"main_license" has to be an existing file
-The exception blocks and the nolicense block have to be pairwise disjoint sets
-The search path is always the current working directory (else the path specifications make no sense)
*/

#include <sge/charconv/utf8_char.hpp>
#include <sge/charconv/utf8_string.hpp>
#include <sge/parse/json/array.hpp>
#include <sge/parse/json/find_member.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/get_exn.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/parse_file_exn.hpp>
#include <sge/parse/json/start.hpp>
#include <sge/parse/json/value.hpp>
#include <fcppt/args_char.hpp>
#include <fcppt/args_from_second.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/main.hpp>
#include <fcppt/make_cref.hpp>
#include <fcppt/recursive_impl.hpp>
#include <fcppt/reference_impl.hpp>
#include <fcppt/strong_typedef_output.hpp> // NOLINT(misc-include-cleaner)
#include <fcppt/text.hpp>
#include <fcppt/algorithm/contains_if.hpp>
#include <fcppt/algorithm/map.hpp>
#include <fcppt/algorithm/map_optional.hpp>
#include <fcppt/either/match.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/clog.hpp>
#include <fcppt/iterator/make_range.hpp>
#include <fcppt/optional/maybe.hpp>
#include <fcppt/optional/object.hpp>
#include <fcppt/options/argument.hpp>
#include <fcppt/options/error.hpp>
#include <fcppt/options/error_output.hpp>
#include <fcppt/options/long_name.hpp>
#include <fcppt/options/optional_help_text.hpp>
#include <fcppt/options/parse.hpp>
#include <fcppt/options/result_of.hpp>
#include <fcppt/options/usage_output.hpp>
#include <fcppt/preprocessor/ignore_dangling_reference.hpp>
#include <fcppt/preprocessor/pop_warning.hpp>
#include <fcppt/preprocessor/push_warning.hpp>
#include <fcppt/record/get.hpp>
#include <fcppt/record/make_label.hpp>
#include <fcppt/config/external_begin.hpp>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <regex>
#include <set>
#include <utility>
#include <vector>
#include <fcppt/config/external_end.hpp>

namespace
{

using regex = std::basic_regex<sge::charconv::utf8_char>;

using regex_set = std::vector<regex>;

using exception = std::pair<regex_set, std::filesystem::path>;

using exception_set = std::vector<exception>;

using path_set = std::set<std::filesystem::path>;

regex_set extract_regexes(sge::parse::json::array const &_array)
{
  return fcppt::algorithm::map<regex_set>(
      _array.elements,
      [](fcppt::recursive<sge::parse::json::value> const &_value)
      {
        return regex( // NOLINT(fuchsia-default-arguments-calls)
            sge::parse::json::get_exn<sge::charconv::utf8_string>(fcppt::make_cref(_value.get()))
                .get());
      });
}

exception_set extract_exceptions(sge::parse::json::object const &_object)
{
  return fcppt::optional::maybe(
      sge::parse::json::find_member<sge::parse::json::array>(
          fcppt::make_cref(_object.members), sge::charconv::utf8_string{"exceptions"}),
      [] { return exception_set(); },
      [](fcppt::reference<sge::parse::json::array const> const &_array)
      {
        return fcppt::algorithm::map<exception_set>(
            _array.get().elements,
            [](fcppt::recursive<sge::parse::json::value> const &_value)
            {
              FCPPT_PP_PUSH_WARNING
              FCPPT_PP_IGNORE_DANGLING_REFERENCE
              sge::parse::json::object const &r2{
                  sge::parse::json::get_exn<sge::parse::json::object>(
                      fcppt::make_cref(_value.get()))
                      .get()};
              FCPPT_PP_POP_WARNING

              std::filesystem::path const license_file( // NOLINT(fuchsia-default-arguments-calls)
                  sge::parse::json::find_member_exn<sge::charconv::utf8_string>(
                      fcppt::make_cref(r2.members), sge::charconv::utf8_string{"license"})
                      .get());

              if (!std::filesystem::exists(license_file))
              {
                throw fcppt::exception{
                    FCPPT_TEXT("License file ") + fcppt::filesystem::path_to_string(license_file) +
                    FCPPT_TEXT(" does not exist!")};
              }

              return exception(
                  extract_regexes(
                      sge::parse::json::find_member_exn<sge::parse::json::array>(
                          fcppt::make_cref(r2.members), sge::charconv::utf8_string{"files"})
                          .get()),
                  license_file);
            });
      });
}

bool has_hidden_components(std::filesystem::path const &_path)
{
  if (_path.empty())
  {
    return false;
  }

  return fcppt::algorithm::contains_if(
      fcppt::iterator::make_range(
          // skip the first entry because it starts with "./"
          std::next( // NOLINT(fuchsia-default-arguments-calls)
              _path.begin()),
          _path.end()),
      [](std::filesystem::path const &_entry)
      { return fcppt::filesystem::path_to_string(_entry).at(0) == FCPPT_TEXT('.'); });
}

path_set extract_paths(regex const &_standard_regex)
{
  return fcppt::algorithm::map_optional<path_set>(
      fcppt::iterator::make_range(
          std::filesystem::recursive_directory_iterator( // NOLINT(fuchsia-default-arguments-calls)
              std::filesystem::path{// NOLINT(fuchsia-default-arguments-calls)
                                    FCPPT_TEXT(".")}),
          std::filesystem::recursive_directory_iterator()),
      [&_standard_regex](std::filesystem::path const &_path)
      {
        using optional_path = fcppt::optional::object<std::filesystem::path>;

        return std::filesystem::is_regular_file(_path) && !has_hidden_components(_path) &&
                       std::regex_match( // NOLINT(fuchsia-default-arguments-calls)
                           _path.string(),
                           _standard_regex)
                   ? optional_path(_path)
                   : optional_path();
      });
}

path_set intersection(path_set const &_paths, regex_set const &_regexs)
{
  path_set ret;

  for (auto const &path : _paths)
  {
    for (auto const &r : _regexs)
    {
      if (path.string().starts_with("./"))
      {
        throw fcppt::exception{FCPPT_TEXT("Path does not start with ./")};
      }

      bool const result = std::regex_match( // NOLINT(fuchsia-default-arguments-calls)
          path.string().substr(2), // NOLINT(fuchsia-default-arguments-calls)
          r,
          std::regex_constants::match_default);

      if (result)
      {
        ret.insert(path);
      }
    }
  }
  return ret;
}

using path_set_with_license = std::pair<path_set, std::filesystem::path>;

using path_set_with_license_set = std::set<path_set_with_license>;

path_set_with_license_set intersections(path_set const &ps, exception_set const &es)
{
  path_set_with_license_set pls{};

  for (auto const &ex : es)
  {
    pls.insert(path_set_with_license(intersection(ps, ex.first), ex.second));
  }
  return pls;
}

// Thanks to Graphics Noob at
// http://stackoverflow.com/questions/1964150/c-test-if-2-sets-are-disjoint
template <class Set1, class Set2>
bool is_disjoint(Set1 const &set1, Set2 const &set2)
{
  if (set1.empty() || set2.empty())
  {
    return true;
  }

  auto it1{set1.begin()};
  auto it1End{set1.end()};

  auto it2{set2.begin()};
  auto it2End{set2.end()};

  if (*it1 > *set2.rbegin() || *it2 > *set1.rbegin())
  {
    return true;
  }

  while (it1 != it1End && it2 != it2End)
  {
    if (*it1 == *it2)
    {
      return false;
    }
    if (*it1 < *it2)
    {
      it1++;
    }
    else
    {
      it2++;
    }
  }

  return true;
}

template <typename T>
bool is_pairwise_disjoint(std::set<T *> const &ts)
{
  if (ts.empty())
  {
    return true;
  }

  for (auto i = ts.begin(); i != std::prev(ts.end()); // NOLINT(fuchsia-default-arguments-calls)
       ++i)
  {
    for (auto j = std::next(i); // NOLINT(fuchsia-default-arguments-calls)
         j != ts.end();
         ++j)
    {
      if (!is_disjoint(**i, **j))
      {
        return false;
      }
    }
  }

  return true;
}

using path_ref_set = std::set<path_set const *>;

path_set ref_set_difference(path_set _paths, path_ref_set const &refs)
{
  for (auto const &path : refs)
  {
    path_set b{};

    std::set_difference(
        _paths.begin(), _paths.end(), path->begin(), path->end(), std::inserter(b, b.begin()));

    _paths.swap(b);
  }
  return _paths;
}

void apply_license(std::filesystem::path const &file, std::filesystem::path const &license)
{
  if (std::system( // NOLINT(cert-env33-c)
          ("update_license " + file.string() + " " + license.string()).c_str()) != EXIT_SUCCESS)
  {
    fcppt::io::cerr() << FCPPT_TEXT("update_license failed for ")
                      << fcppt::filesystem::path_to_string(file) << FCPPT_TEXT('\n');
  }
}

int main_function(sge::parse::json::start const &json_file)
{
  sge::parse::json::object const &json_object(json_file.object());

  std::filesystem::path const main_license{
      // NOLINT(fuchsia-default-arguments-calls)
      sge::parse::json::find_member_exn<sge::charconv::utf8_string>(
          fcppt::make_cref(json_object.members), sge::charconv::utf8_string{"main_license"})
          .get()};

  regex const standard_regex( // NOLINT(fuchsia-default-arguments-calls)
      sge::parse::json::find_member_exn<sge::charconv::utf8_string>(
          fcppt::make_cref(json_object.members), sge::charconv::utf8_string{"standard_match"})
          .get());

  if (!std::filesystem::exists(main_license))
  {
    throw fcppt::exception{
        FCPPT_TEXT("main license file ") + fcppt::filesystem::path_to_string(main_license) +
        FCPPT_TEXT(" does not exist")};
  }

  exception_set const exceptions = extract_exceptions(json_object);

  regex_set const nolicense{extract_regexes(
      sge::parse::json::find_member_exn<sge::parse::json::array>(
          fcppt::make_cref(json_object.members), sge::charconv::utf8_string{"nolicense"})
          .get())};

  path_set const a = extract_paths(standard_regex);

  path_set const nolicense_paths = intersection(a, nolicense);

  path_set_with_license_set const exception_paths = intersections(a, exceptions);

  path_ref_set path_refs;

  for (auto const &ref : exception_paths)
  {
    path_refs.insert(&(ref.first));
  }

  path_refs.insert(&nolicense_paths);

  if (!is_pairwise_disjoint(path_refs))
  {
    fcppt::io::cerr() << FCPPT_TEXT(
        "The nolicense and exception sets have to be pairwise disjoint. They're not.\n");
    return EXIT_FAILURE;
  }

  // Collect all the files which are not "excepted" and have a license
  {
    path_set const difference_set(ref_set_difference(a, path_refs));

    for (auto const &path : difference_set)
    {
      apply_license(path, main_license);
    }
  }

  // Collect all exceptions
  for (auto const &ex : exception_paths)
  {
    for (auto const &path : ex.first)
    {
      apply_license(path, ex.second);
    }
  }

  return EXIT_SUCCESS;
}

}

int FCPPT_MAIN(int const argc, fcppt::args_char *argv[])
try
{
  FCPPT_RECORD_MAKE_LABEL(file_label);

  fcppt::options::argument<file_label, std::filesystem::path> const parser{
      fcppt::options::long_name{FCPPT_TEXT("json-file")}, fcppt::options::optional_help_text{}};

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
        std::filesystem::path const &json_file_name{fcppt::record::get<file_label>(_result)};

        sge::parse::json::start const json{sge::parse::json::parse_file_exn(json_file_name)};

        fcppt::io::clog() << FCPPT_TEXT("Successfully parsed the file \"") << json_file_name
                          << FCPPT_TEXT("\"\n");

        return main_function(json);
      });
}
catch (fcppt::exception const &_e)
{
  fcppt::io::cerr() << FCPPT_TEXT("Caught an exception: ") << _e.string() << FCPPT_TEXT("\n");

  return EXIT_FAILURE;
}
catch (std::exception const &_e)
{
  std::cerr << "Caught a standard exception: " << _e.what() << '\n';

  return EXIT_FAILURE;
}
