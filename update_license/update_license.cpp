#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <iosfwd>
#include <cstdlib>
#include <cstdio>

int
main(
	int argc,
	char **argv
)
{
	if(
		argc != 3
	)
	{
		std::cerr
			<< "Usage: "
			<< argv[0]
			<< " <file> <license_file>\n";

		return EXIT_FAILURE;
	}

	std::string const filename(
		argv[1]
	);

	std::ifstream ifs(
		filename.c_str()
	);

	if(
		!ifs.is_open()
	)
	{
		std::cerr
			<< "Cannot open "
			<< filename
			<< '\n';

		return EXIT_FAILURE;
	}

	{
		namespace encoding = boost::spirit::qi::standard;

		using boost::spirit::repository::qi::confix;
		using boost::spirit::qi::eol;
		using encoding::char_;

		ifs
			>> std::noskipws
			>> boost::spirit::qi::match(
				*(
					confix(
						"/*",
						"*/"
					)[
						*(char_ - "*/")
					]
					|
					confix(
						"//",
						eol
					)[
						*(char_ - eol)
					]
				)
				>> *eol
			);
	}

	ifs.unget(); // FIXME

	if(
		!ifs.good()
	)
	{
		std::cerr
			<< "Input file "
			<< filename
			<< " broken\n";

		return EXIT_FAILURE;
	}

	std::string const license_filename(
		argv[2]
	);

	std::ifstream license(
		license_filename.c_str()
	);

	if(
		!license.is_open()
	)
	{
		std::cerr
			<< "Cannot open license file "
			<< license_filename
			<< '\n';

		return EXIT_FAILURE;
	}

	std::string const temp_filename(
		filename + ".temp"
	);

	{
		std::ofstream ofs(
			temp_filename.c_str()
		);

		if(
			!ofs.is_open()
		)
		{
			std::cerr
				<< "Cannot open "
				<< temp_filename
				<< '\n';

			return EXIT_FAILURE;
		}

		ofs
			<< license.rdbuf()
			<< ifs.rdbuf();
	}

	{
		ifs.clear();

		ifs.seekg(
			std::ios_base::beg
		);

		if(
			!ifs
		)
		{
			std::cerr
				<< "Rewinding "
				<< filename
				<< " failed\n";

			return EXIT_FAILURE;
		}

		std::ifstream new_ifs(
			temp_filename.c_str()
		);

		if(
			!new_ifs
		)
		{
			std::cerr
				<< "Opening "
				<< temp_filename
				<< " failed\n";

			return EXIT_FAILURE;
		}

		std::string
			line1,
			line2;

		bool equal(
			true
		);

		while(
			std::getline(
				ifs,
				line1
			)
			&& std::getline(
				new_ifs,
				line2
			)
		)
			if(
				line1 != line2
			)
			{
				equal = false;

				break;
			}

		if(
			equal
		)
		{
			new_ifs.close();

			if(
				std::remove(
					temp_filename.c_str()
				)
				!= 0
			)
			{
				std::cerr
					<< "Cannot remove "
					<< temp_filename
					<< '\n';

				return EXIT_FAILURE;
			}

			return EXIT_SUCCESS;
		}
	}

	ifs.close();

	if(
		std::rename(
			temp_filename.c_str(),
			filename.c_str()
		)
		!= 0
	)
	{
		std::cerr
			<< "Cannot rename "
			<< temp_filename
			<< " into "
			<< filename
			<< '\n';

		if(
			std::remove(
				temp_filename.c_str()
			)
			!= 0
		)
			std::cerr
				<< "Cannot remove "
				<< temp_filename
				<< "! You have to remove it manually!\n";

		return EXIT_FAILURE;
	}
}
