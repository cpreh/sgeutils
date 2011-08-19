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

#include <sge/parse/json/parse_file.hpp>
#include <sge/parse/json/find_member_exn.hpp>
#include <sge/parse/json/find_member.hpp>
#include <sge/parse/json/object.hpp>
#include <sge/parse/json/element_vector.hpp>
#include <sge/parse/json/array.hpp>
#include <fcppt/io/cerr.hpp>
#include <fcppt/io/clog.hpp>
#include <fcppt/text.hpp>
#include <fcppt/exception.hpp>
#include <fcppt/from_std_string.hpp>
#include <fcppt/to_std_string.hpp>
#include <fcppt/assert/exception.hpp>
#include <fcppt/assert/throw.hpp>
#include <fcppt/filesystem/recursive_directory_iterator.hpp>
#include <fcppt/filesystem/exists.hpp>
#include <fcppt/filesystem/is_regular.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/next_prior.hpp>
#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <exception>
#include <iterator>
#include <utility>
#include <cstdlib>

namespace
{
typedef
boost::basic_regex<fcppt::char_type>
regex;

typedef
std::set
<
	regex
>
regex_set;

typedef
std::pair
<
	regex_set,
	fcppt::filesystem::path
>
exception;

typedef
std::set
<
	exception
>
exception_set;

typedef
std::set<fcppt::filesystem::path>
path_set;

regex_set const
extract_regexes(
	sge::parse::json::array const &a)
{
	regex_set rs;
	BOOST_FOREACH(
		sge::parse::json::element_vector::const_reference r,
		a.elements)
		rs.insert(
			regex(
				sge::parse::json::get<sge::parse::json::string>(
					r)));
	return rs;
}

exception_set const
extract_exceptions(
	sge::parse::json::object const &o)
{
	exception_set es;

	sge::parse::json::array const *const exceptions_(
		sge::parse::json::find_member<
			sge::parse::json::array
		>(
			o.members,
			FCPPT_TEXT("exceptions")
		)
	);

	if(
		!exceptions_
	)
		return es;


	BOOST_FOREACH(
		sge::parse::json::element_vector::const_reference r1,
		exceptions_->elements
	)
	{
		sge::parse::json::object const &r2 =
			sge::parse::json::get<sge::parse::json::object>(
				r1);

		es.insert(
			exception(
				extract_regexes(
					sge::parse::json::find_member_exn<sge::parse::json::array>(
						r2.members,
						FCPPT_TEXT("files"))),
				sge::parse::json::find_member_exn<sge::parse::json::string>(
					r2.members,
					FCPPT_TEXT("license"))));

		// The license file has to exist!
		FCPPT_ASSERT_THROW(
			fcppt::filesystem::exists(
				(--es.end())->second
			),
			fcppt::assert_::exception
		)
	}
	return es;
}

bool
has_hidden_components(
	fcppt::filesystem::path const &path_
)
{
	if(path_.empty())
		return false;

	// skip the first entry because it starts with "./"
	for(
		fcppt::filesystem::path::iterator next(
			boost::next(
				path_.begin()
			)
		);
		next != path_.end();
		++next
	)
		if(
			next->string().at(0) == FCPPT_TEXT('.')
		)
			return true;

	return false;
}

path_set const
extract_paths(
	regex const &_standard_regex)
{
	path_set s;
	for(
		fcppt::filesystem::recursive_directory_iterator
			i(
				FCPPT_TEXT(".")),
			end;
		i != end;
		++i
	)
	{
		fcppt::filesystem::path const &path_(
			i->path()
		);

		if(
			fcppt::filesystem::is_regular(
				path_
			)
			&&
			!has_hidden_components(
				path_
			)
			&&
			boost::regex_match(
				path_.string(),
				_standard_regex)
		)
			s.insert(
				path_
			);
	}
	return s;
}

path_set const
intersection(
	path_set const &ps,
	regex_set const &rs)
{
	path_set ret;
	BOOST_FOREACH(
		path_set::const_reference p,
		ps)
		BOOST_FOREACH(
			regex_set::const_reference r,
			rs)
		{
			FCPPT_ASSERT_THROW(
				p.string().compare(0,2,"./") == 0,
				fcppt::assert_::exception
			);

			bool const result =
				regex_match(
					p.string().substr(2),
					r,
					boost::match_default);

			if(result)
				ret.insert(
					p);
		}
	return ret;
}

typedef
std::pair
<
	path_set,
	fcppt::filesystem::path
>
path_set_with_license;

typedef
std::set<path_set_with_license>
path_set_with_license_set;

path_set_with_license_set const
intersections(
	path_set const &ps,
	exception_set const &es)
{
	path_set_with_license_set pls;
	BOOST_FOREACH(
		exception_set::const_reference e,
		es)
	{
		pls.insert(
			path_set_with_license(
				intersection(
					ps,
					e.first),
				e.second));
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

	for(
		const_iterator i = ts.begin();
		i != boost::prior(ts.end());
		++i)
		for (
			const_iterator j = boost::next(i);
			j != ts.end();
			++j)
			if (!is_disjoint(**i,**j))
				return false;
	return true;
}


typedef
std::set<path_set const *>
path_ref_set;

path_set const
ref_set_difference(
	path_set a,
	path_ref_set const &refs)
{
	BOOST_FOREACH(
		path_ref_set::const_reference r,
		refs)
	{
		path_set b;
		std::set_difference(
			a.begin(),
			a.end(),
			r->begin(),
			r->end(),
			std::inserter(
				b,
				b.begin()));
		// H4x0r!
		a.swap(
			b);
	}
	return a;
}

void
apply_license(
	fcppt::filesystem::path const &file,
	fcppt::filesystem::path const &license
)
{
	if(
		std::system(
			(
				"update_license "
				+ fcppt::to_std_string(
					file.string()
				)
				+ " "
				+ fcppt::to_std_string(
					license.string()
				)
			).c_str()
		)
		!= EXIT_SUCCESS
	)
		throw fcppt::exception(
			FCPPT_TEXT("update_license returned an error")
		);
}

}

int main(
	int const _argc,
	char *_argv[])
try
{
	if (_argc != 2)
	{
		fcppt::io::cerr
			<< FCPPT_TEXT("usage: ")
			<< fcppt::from_std_string(_argv[0])
			<< FCPPT_TEXT(" <json file>\n");
		return EXIT_FAILURE;
	}

	fcppt::string const json_file_name =
		fcppt::from_std_string(
			_argv[1]);

	sge::parse::json::object json_file;

	if (
		!sge::parse::json::parse_file(
			json_file_name,
			json_file))
	{
		fcppt::io::cerr
			<< FCPPT_TEXT("Couldn't parse file \"")
			<< json_file_name
			<< FCPPT_TEXT("\"\n");
		return EXIT_FAILURE;
	}

	fcppt::io::clog
		<< FCPPT_TEXT("Successfully parsed the file \"")
		<< json_file_name
		<< FCPPT_TEXT("\"\n");

	fcppt::filesystem::path const main_license =
		sge::parse::json::find_member_exn<sge::parse::json::string>(
			json_file.members,
			FCPPT_TEXT("main_license"));

	regex const standard_regex(
		sge::parse::json::find_member_exn<sge::parse::json::string>(
			json_file.members,
			FCPPT_TEXT("standard_match")));

	FCPPT_ASSERT_THROW(
		fcppt::filesystem::exists(
			main_license
		),
		fcppt::assert_::exception
	)

	exception_set const exceptions =
		extract_exceptions(
			json_file);

	regex_set const nolicense =
		extract_regexes(
			sge::parse::json::find_member_exn<sge::parse::json::array>(
				json_file.members,
				FCPPT_TEXT("nolicense")));

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
	BOOST_FOREACH(
		path_set_with_license_set::const_reference r,
		exception_paths)
		path_refs.insert(
			&(r.first));

	path_refs.insert(
		&nolicense_paths);

	if (!is_pairwise_disjoint(path_refs))
	{
		fcppt::io::cerr << FCPPT_TEXT("The nolicense and exception sets have to be pairwise disjoint. They're not.\n");
		return EXIT_FAILURE;
	}

	// Collect all the files which are not "excepted" and have a license
	BOOST_FOREACH(
		path_set::const_reference r,
		ref_set_difference(
			a,
			path_refs
		)
	)
		apply_license(
			r,
			main_license
		);

	// Collect all exceptions
	BOOST_FOREACH(
		path_set_with_license_set::const_reference r,
		exception_paths)
		BOOST_FOREACH(path_set::const_reference p,r.first)
			apply_license(
				p,
				r.second
			);
}
catch (
	fcppt::exception const &_e)
{
	fcppt::io::cerr
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
