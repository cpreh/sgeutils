#include <sge/charconv/utf8_string.hpp>
#include <sge/charconv/utf8_string_to_fcppt.hpp>
#include <sge/parse/json/array.hpp>
#include <sge/parse/json/element_vector.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/parse_file_exn.hpp>
#include <sge/parse/json/start.hpp>
#include <fcppt/args_char.hpp>
#include <fcppt/args_from_second.hpp>
#include <fcppt/char_type.hpp>
#include <fcppt/copy.hpp>
#include <fcppt/declare_strong_typedef.hpp>
#include <fcppt/deref_reference.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/main.hpp>
#include <fcppt/make_cref.hpp>
#include <fcppt/make_int_range_count.hpp>
#include <fcppt/make_ref.hpp>
#include <fcppt/output_to_fcppt_string.hpp>
#include <fcppt/reference_impl.hpp>
#include <fcppt/string.hpp>
#include <fcppt/strong_typedef_impl.hpp>
#include <fcppt/strong_typedef_output.hpp>
#include <fcppt/text.hpp>
#include <fcppt/to_std_string.hpp>
#include <fcppt/algorithm/contains.hpp>
#include <fcppt/algorithm/join_strings.hpp>
#include <fcppt/algorithm/map.hpp>
#include <fcppt/algorithm/map_optional.hpp>
#include <fcppt/container/join.hpp>
#include <fcppt/config/platform.hpp>
#include <fcppt/either/match.hpp>
#include <fcppt/either/output.hpp>
#include <fcppt/either/to_exception.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/cout.hpp>
#include <fcppt/optional/make.hpp>
#include <fcppt/optional/to_exception.hpp>
#include <fcppt/options/apply.hpp>
#include <fcppt/options/error.hpp>
#include <fcppt/options/long_name.hpp>
#include <fcppt/options/make_default_value.hpp>
#include <fcppt/options/option.hpp>
#include <fcppt/options/optional_help_text.hpp>
#include <fcppt/options/optional_short_name.hpp>
#include <fcppt/options/parse.hpp>
#include <fcppt/options/result_of.hpp>
#include <fcppt/options/short_name.hpp>
#include <fcppt/options/switch.hpp>
#include <fcppt/parse/basic_char_set.hpp>
#include <fcppt/parse/basic_string.hpp>
#include <fcppt/parse/convert_const.hpp>
#include <fcppt/parse/make_fatal.hpp>
#include <fcppt/parse/make_lexeme.hpp>
#include <fcppt/parse/parse_string_error.hpp>
#include <fcppt/parse/parse_string_error_output.hpp>
#include <fcppt/parse/phrase_parse_string.hpp>
#include <fcppt/parse/space_set.hpp>
#include <fcppt/parse/operators/alternative.hpp>
#include <fcppt/parse/operators/complement.hpp>
#include <fcppt/parse/operators/repetition_plus.hpp>
#include <fcppt/parse/operators/sequence.hpp>
#include <fcppt/parse/skipper/basic_space.hpp>
#include <fcppt/preprocessor/disable_gcc_warning.hpp>
#include <fcppt/preprocessor/pop_warning.hpp>
#include <fcppt/preprocessor/push_warning.hpp>
#include <fcppt/record/get.hpp>
#include <fcppt/record/make_label.hpp>
#include <fcppt/tuple/object_impl.hpp>
#include <fcppt/variant/match.hpp>
#include <fcppt/variant/object_impl.hpp>
#include <fcppt/variant/output.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/asio/io_service.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <vector>
#include <fcppt/config/external_end.hpp>

namespace
{

fcppt::string make_syntax_only(fcppt::string const &_compile_command)
{
  auto const word{fcppt::parse::make_lexeme(+~fcppt::parse::basic_char_set<fcppt::char_type>{
      fcppt::parse::space_set<fcppt::char_type>()})};

  using string_sequence = std::vector<fcppt::string>;

  struct otag{};

  return fcppt::algorithm::join_strings(
      fcppt::container::join(
          fcppt::algorithm::map_optional<string_sequence>(
              fcppt::either::to_exception(
                  fcppt::parse::phrase_parse_string(
                      +((fcppt::parse::convert_const{
                             fcppt::parse::basic_string<fcppt::char_type>{FCPPT_TEXT("-o")},
                             otag{}} >>
                         fcppt::parse::make_fatal(fcppt::make_cref(word))) |
                        fcppt::make_cref(word)),
                      fcppt::copy(_compile_command),
                      fcppt::parse::skipper::basic_space<fcppt::char_type>()),
                  [](fcppt::parse::parse_string_error<fcppt::char_type> const &_error)
                  {
                    return fcppt::exception{
                        FCPPT_TEXT("Failed to parse command line: ") +
                        fcppt::output_to_fcppt_string(_error)};
                  }),
              [](fcppt::variant::object<
                  fcppt::tuple::object<otag, fcppt::string>,
                  fcppt::string> const &_element)
              {
                return fcppt::variant::match(
                    _element,
                    [](fcppt::tuple::object<otag, fcppt::string> const &)
                    { return fcppt::optional::object<fcppt::string>{}; },
                    [](fcppt::string const &_string)
                    { return fcppt::optional::make(_string); });
              }),
          string_sequence{FCPPT_TEXT("-fsyntax-only")}),
      FCPPT_TEXT(" "));
}

FCPPT_DECLARE_STRONG_TYPEDEF(bool, verbose);

void worker(
    sge::parse::json::value const &_element,
    fcppt::reference<boost::asio::io_service> const _io_service,
    verbose const _verbose)
{
  sge::parse::json::object const &json_object(
      sge::parse::json::get_exn<sge::parse::json::object>(fcppt::make_cref(_element)).get());

  {
    std::filesystem::path const filename( // NOLINT(fuchsia-default-arguments-calls)
        sge::parse::json::find_member_exn<sge::charconv::utf8_string const>(
            fcppt::make_cref(json_object.members), sge::charconv::utf8_string{"file"})
            .get());

    if (fcppt::algorithm::contains(
            filename,
            std::filesystem::path( // NOLINT(fuchsia-default-arguments-calls)
                FCPPT_TEXT("impl"))) ||
        filename.extension() != std::filesystem::path{// NOLINT(fuchsia-default-arguments-calls)
                                                      ".hpp"})
    {
      return;
    }
  }

  std::string const command{fcppt::optional::to_exception(
      fcppt::to_std_string(make_syntax_only(fcppt::optional::to_exception(
          sge::charconv::utf8_string_to_fcppt(
              sge::parse::json::find_member_exn<sge::charconv::utf8_string const>(
                  fcppt::make_cref(json_object.members), sge::charconv::utf8_string{"command"})
                  .get()),
          [] { return fcppt::exception{FCPPT_TEXT("Failed to convert command!")}; }))),
      [] { return fcppt::exception{FCPPT_TEXT("Failed to convert command!")}; })};

  if (_verbose.get())
  {
    std::cout << command << '\n';
  }

  int const exit_status(std::system( // NOLINT(cert-env33-c)
      command.c_str()));

  if (exit_status == -1)
  {
    fcppt::io::cerr() << FCPPT_TEXT("system() failed\n");

    _io_service.get().stop();

    return;
  }
#if defined(FCPPT_CONFIG_POSIX_PLATFORM)
  FCPPT_PP_PUSH_WARNING
  FCPPT_PP_DISABLE_GCC_WARNING(-Wcast-qual)
  FCPPT_PP_DISABLE_GCC_WARNING(-Wold-style-cast)
  if (WIFSIGNALED( // NOLINT(hicpp-signed-bitwise)
          exit_status))
    FCPPT_PP_POP_WARNING
    {
      fcppt::io::cout() << FCPPT_TEXT('\n');

      _io_service.get().stop();
    }
#endif
}

FCPPT_DECLARE_STRONG_TYPEDEF(unsigned, num_threads);

void main_program(num_threads const _num_threads, verbose const _verbose)
{
  fcppt::io::cout() << FCPPT_TEXT("Starting ") << _num_threads << FCPPT_TEXT(" threads\n");

  sge::parse::json::array const build_commands(
      sge::parse::json::parse_file_exn(
          std::filesystem::path{// NOLINT(fuchsia-default-arguments-calls)
                                FCPPT_TEXT("compile_commands.json")})
          .array());

  boost::asio::io_service io_service{};

  for (auto const &element : build_commands.elements)
  {
    io_service.post([&io_service, element, _verbose]
                    { worker(element.get(), fcppt::make_ref(io_service), _verbose); });
  }

  using thread_vector = std::vector<std::thread>;

  thread_vector threads{fcppt::algorithm::map<thread_vector>(
      fcppt::make_int_range_count(_num_threads.get()),
      [&io_service](unsigned) { return std::thread{[&io_service]() { io_service.run(); }}; })};

  for (auto &thread : threads)
  {
    thread.join();
  }
}

}

FCPPT_PP_PUSH_WARNING
FCPPT_PP_DISABLE_GCC_WARNING(-Wmissing-declarations)

int FCPPT_MAIN(int argc, fcppt::args_char **argv)
try
{
  FCPPT_RECORD_MAKE_LABEL(verbose_label);

  FCPPT_RECORD_MAKE_LABEL(num_threads_label);

  auto const parser(fcppt::options::apply(
      fcppt::options::switch_<verbose_label>{
          fcppt::options::optional_short_name{fcppt::options::short_name{FCPPT_TEXT("v")}},
          fcppt::options::long_name{FCPPT_TEXT("verbose")},
          fcppt::options::optional_help_text{}},
      fcppt::options::option<num_threads_label, unsigned>{
          fcppt::options::optional_short_name{fcppt::options::short_name{FCPPT_TEXT("j")}},
          fcppt::options::long_name{FCPPT_TEXT("threads")},
          fcppt::options::make_default_value(
              fcppt::optional::make(std::max(std::thread::hardware_concurrency(), 1U))),
          fcppt::options::optional_help_text{}}));

  return fcppt::either::match(
      fcppt::options::parse(parser, fcppt::args_from_second(argc, argv)),
      [&parser](fcppt::options::error const &_error)
      {
        fcppt::io::cerr() << _error << FCPPT_TEXT('\n') << parser.usage() << FCPPT_TEXT('\n');

        return EXIT_FAILURE;
      },
      [](fcppt::options::result_of<decltype(parser)> const &_args)
      {
        main_program(
            num_threads{fcppt::record::get<num_threads_label>(_args)},
            verbose{fcppt::record::get<verbose_label>(_args)});

        return EXIT_SUCCESS;
      });

  return EXIT_SUCCESS;
}
catch (fcppt::exception const &_exception)
{
  fcppt::io::cerr() << _exception.string() << FCPPT_TEXT('\n');

  return EXIT_FAILURE;
}
catch (std::exception const &_exception)
{
  std::cerr << _exception.what() << '\n';

  return EXIT_FAILURE;
}

FCPPT_PP_POP_WARNING
