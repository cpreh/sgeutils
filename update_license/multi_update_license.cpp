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
#include <sge/parse/json/get_exn.hpp>
#include <sge/parse/json/find_member.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/parse_file_exn.hpp>
#include <sge/parse/json/start.hpp>
#include <sge/parse/json/value.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/recursive_impl.hpp>
#include <fcppt/reference_impl.hpp>
#include <fcppt/optional/maybe.hpp>
#include <fcppt/optional/object.hpp>
#include <fcppt/strong_typedef_output.hpp>
#include <fcppt/text.hpp>
#include <fcppt/algorithm/contains_if.hpp>
#include <fcppt/algorithm/map.hpp>
#include <fcppt/algorithm/map_optional.hpp>
#include <fcppt/assert/exception.hpp>
#include <fcppt/assert/throw.hpp>
#include <fcppt/filesystem/path_to_string.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/clog.hpp>
#include <fcppt/config/external_begin.hpp>
#include <boost/filesystem/operations.hpp>
#include <filesystem>
#include <boost/range/iterator_range_core.hpp>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <ostream>
#include <regex>
#include <set>
#include <utility>
#include <vector>
#include <fcppt/config/external_end.hpp>


namespace
{

typedef
std::basic_regex<
	sge::charconv::utf8_char
>
regex;

typedef
std::vector<
	regex
>
regex_set;

typedef
std::pair<
	regex_set,
	std::filesystem::path
>
exception;

typedef
std::vector<
	exception
>
exception_set;

typedef
std::set<
	std::filesystem::path
>
path_set;

regex_set
extract_regexes(
	sge::parse::json::array const &_array
)
{
	return
		fcppt::algorithm::map<
			regex_set
		>(
			_array.elements,
			[](
				fcppt::recursive<
					sge::parse::json::value
				> const &_value
			)
			{
				return
					regex(
						sge::parse::json::get_exn<
							sge::charconv::utf8_string
						>(
							_value.get()
						)
					);
			}
		);
}

exception_set
extract_exceptions(
	sge::parse::json::object const &_object
)
{
	return
		fcppt::optional::maybe(
			sge::parse::json::find_member<
				sge::parse::json::array
			>(
				_object.members,
				sge::charconv::utf8_string{
					"exceptions"
				}
			),
			[]{
				return
					exception_set();
			},
			[](
				fcppt::reference<
					sge::parse::json::array const
				> const &_array
			)
			{
				return
					fcppt::algorithm::map<
						exception_set
					>(
						_array.get().elements,
						[](
							fcppt::recursive<
								sge::parse::json::value
							> const &_value
						)
						{
							sge::parse::json::object const &r2(
								sge::parse::json::get_exn<
									sge::parse::json::object
								>(
									_value.get()
								)
							);

							std::filesystem::path license_file(
								sge::parse::json::find_member_exn<
									sge::charconv::utf8_string
								>(
									r2.members,
									sge::charconv::utf8_string{
										"license"
									}
								)
							);

							FCPPT_ASSERT_THROW(
								boost::filesystem::exists(
									license_file
								),
								fcppt::assert_::exception
							);

							return
								exception(
									extract_regexes(
										sge::parse::json::find_member_exn<
											sge::parse::json::array
										>(
											r2.members,
											sge::charconv::utf8_string{
												"files"
											}
										)
									),
									license_file
								);
						}
					);
			}
		);
}

bool
has_hidden_components(
	std::filesystem::path const &_path
)
{
	if(
		_path.empty()
	)
		return
			false;

	return
		fcppt::algorithm::contains_if(
			boost::make_iterator_range(
				// skip the first entry because it starts with "./"
				std::next(
					_path.begin()
				),
				_path.end()
			),
			[](
				std::filesystem::path const &_entry
			)
			{
				return
					fcppt::filesystem::path_to_string(
						_entry
					).at(
						0
					)
					==
					FCPPT_TEXT('.');
			}
		);
}

path_set
extract_paths(
	regex const &_standard_regex
)
{
	return
		fcppt::algorithm::map_optional<
			path_set
		>(
			boost::make_iterator_range(
				boost::filesystem::recursive_directory_iterator(
					FCPPT_TEXT(".")
				)
			),
			[
				&_standard_regex
			](
				std::filesystem::path const &_path
			)
			{
				typedef
				fcppt::optional::object<
					std::filesystem::path
				>
				optional_path;

				return
					boost::filesystem::is_regular_file(
						_path
					)
					&&
					!has_hidden_components(
						_path
					)
					&&
					std::regex_match(
						_path.string(),
						_standard_regex
					)
					?
						optional_path(
							_path
						)
					:
						optional_path()
					;
			}
		);
}

path_set
intersection(
	path_set const &_paths,
	regex_set const &_regexs)
{
	path_set ret;

	for(
		path_set::const_iterator path_it(
			_paths.begin()
		);
		path_it != _paths.end();
		++path_it
	)
		for(
			regex_set::const_iterator regex_it(
				_regexs.begin()
			);
			regex_it != _regexs.end();
			++regex_it
		)
		{
			FCPPT_ASSERT_THROW(
				path_it->string().compare(0,2,"./") == 0,
				fcppt::assert_::exception
			);

			bool const result =
				regex_match(
					path_it->string().substr(2),
					*regex_it,
					std::regex_constants::match_default);

			if(result)
				ret.insert(
					*path_it);
		}
	return ret;
}

typedef
std::pair
<
	path_set,
	std::filesystem::path
>
path_set_with_license;

typedef
std::set<path_set_with_license>
path_set_with_license_set;

path_set_with_license_set
intersections(
	path_set const &ps,
	exception_set const &es)
{
	path_set_with_license_set pls;

	for(
		exception_set::const_iterator ex_it(
			es.begin()
		);
		ex_it != es.end();
		++ex_it
	)
	{
		pls.insert(
			path_set_with_license(
				intersection(
					ps,
					ex_it->first),
				ex_it->second));
	}
	return pls;
}

// Thanks to Graphics Noob at
// http://stackoverflow.com/questions/1964150/c-test-if-2-sets-are-disjoint
template<class Set1, class Set2>
bool
is_disjoint(
	Set1 const &set1,
	Set2 const &set2)
{
	if(set1.empty() || set2.empty())
		return true;

	typename Set1::const_iterator
		it1 = set1.begin(),
		it1End = set1.end();

	typename Set2::const_iterator
		it2 = set2.begin(),
		it2End = set2.end();

	if(*it1 > *set2.rbegin() || *it2 > *set1.rbegin())
		return true;

	while(it1 != it1End && it2 != it2End)
	{
		if(*it1 == *it2)
			return false;
		if(*it1 < *it2)
			it1++;
		else
			it2++;
	}

	return true;
}


template<typename T>
bool
is_pairwise_disjoint(
	std::set<T*> const &ts)
{
	typedef
	std::set<T*>
	set;

	typedef typename
	set::const_iterator
	const_iterator;

	// TODO: This is wrong
	for(
		const_iterator i = ts.begin();
		i != std::prev(ts.end());
		++i)
		for (
			const_iterator j = std::next(i);
			j != ts.end();
			++j)
			if (!is_disjoint(**i,**j))
				return false;
	return true;
}


typedef
std::set<path_set const *>
path_ref_set;

path_set
ref_set_difference(
	path_set _paths,
	path_ref_set const &refs)
{
	for(
		path_ref_set::const_iterator it(
			refs.begin()
		);
		it != refs.end();
		++it
	)
	{
		path_set b;

		std::set_difference(
			_paths.begin(),
			_paths.end(),
			(*it)->begin(),
			(*it)->end(),
			std::inserter(
				b,
				b.begin()));
		// H4x0r!
		_paths.swap(
			b);
	}
	return _paths;
}

void
apply_license(
	std::filesystem::path const &file,
	std::filesystem::path const &license
)
{
	if(
		std::system(
			(
				"update_license "
				+
				file.string()
				+
				" "
				+
				license.string()
			).c_str()
		)
		!= EXIT_SUCCESS
	)
		fcppt::io::cerr()
			<< FCPPT_TEXT("update_license failed for ")
			<< fcppt::filesystem::path_to_string(
				file
			)
			<< FCPPT_TEXT('\n');
}

int
main_function(
	sge::parse::json::start const &json_file
)
{
	sge::parse::json::object const &json_object(
		json_file.object());

	std::filesystem::path const main_license{
		sge::parse::json::find_member_exn<
			sge::charconv::utf8_string
		>(
			json_object.members,
			sge::charconv::utf8_string{
				"main_license"
			}
		)
	};

	regex const standard_regex(
		sge::parse::json::find_member_exn<
			sge::charconv::utf8_string
		>(
			json_object.members,
			sge::charconv::utf8_string{
				"standard_match"
			}
		)
	);

	FCPPT_ASSERT_THROW(
		boost::filesystem::exists(
			main_license
		),
		fcppt::assert_::exception
	);

	exception_set const exceptions =
		extract_exceptions(
			json_object);

	regex_set const nolicense{
		extract_regexes(
			sge::parse::json::find_member_exn<
				sge::parse::json::array
			>(
				json_object.members,
				sge::charconv::utf8_string{
					"nolicense"
				}
			)
		)
	};

	path_set const a =
		extract_paths(
			standard_regex);

	path_set const nolicense_paths =
		intersection(
			a,
			nolicense);

	path_set_with_license_set const exception_paths =
		intersections(
			a,
			exceptions);

	path_ref_set path_refs;

	for(
		path_set_with_license_set::const_iterator ref_it(
			exception_paths.begin()
		);
		ref_it != exception_paths.end();
		++ref_it
	)
		path_refs.insert(
			&(ref_it->first));

	path_refs.insert(
		&nolicense_paths);

	if (!is_pairwise_disjoint(path_refs))
	{
		fcppt::io::cerr() << FCPPT_TEXT("The nolicense and exception sets have to be pairwise disjoint. They're not.\n");
		return EXIT_FAILURE;
	}

	// Collect all the files which are not "excepted" and have a license
	{
		path_set const difference_set(
			ref_set_difference(
				a,
				path_refs
			)
		);

		for(
			path_set::const_iterator path_it(
				difference_set.begin()
			);
			path_it != difference_set.end();
			++path_it
		)
			apply_license(
				*path_it,
				main_license
			);
	}

	// Collect all exceptions
	for(
		path_set_with_license_set::const_iterator ex_it(
			exception_paths.begin()
		);
		ex_it != exception_paths.end();
		++ex_it
	)
		for(
			path_set::const_iterator path_it(
				ex_it->first.begin()
			);
			path_it != ex_it->first.end();
			++path_it
		)
			apply_license(
				*path_it,
				ex_it->second
			);

	return
		EXIT_SUCCESS;
}

}

int main(
	int const _argc,
	char *_argv[])
try
{
	if (_argc != 2)
	{
		fcppt::io::cerr()
			<< FCPPT_TEXT("usage: ")
			<< fcppt::from_std_string(_argv[0])
			<< FCPPT_TEXT(" <json file>\n");
		return EXIT_FAILURE;
	}

	fcppt::string const json_file_name =
		fcppt::from_std_string(
			_argv[1]);

	sge::parse::json::start const result{
		sge::parse::json::parse_file_exn(
			json_file_name
		)
	};

	fcppt::io::clog()
		<< FCPPT_TEXT("Successfully parsed the file \"")
		<< json_file_name
		<< FCPPT_TEXT("\"\n");

	return
		main_function(
			result
		);
}
catch (
	fcppt::exception const &_e)
{
	fcppt::io::cerr()
		<< FCPPT_TEXT("Caught an exception: ")
		<< _e.string()
		<< FCPPT_TEXT("\n");

	return EXIT_FAILURE;
}
catch (
	std::exception const &_e)
{
	std::cerr
		<< "Caught a standard exception: "
		<< _e.what()
		<< '\n';

	return EXIT_FAILURE;
}
