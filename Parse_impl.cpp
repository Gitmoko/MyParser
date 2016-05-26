#include <iostream>
#include <string>
#include <cmath>
#include <map>
#include <vector>

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3 

#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


#include"MyParserParse.h"

namespace MyParser {
	

	//constexpr auto  Op = operators::sub;
	template<operators Op>
	auto make_binray_operator() {
		return boost::phoenix::bind(
			[](auto const lhs, auto const rhs) {return binary_operator<Op>{lhs, rhs}; }
		, boost::spirit::qi::_val, boost::spirit::qi::_1);
	}

	template<operators Op>
	auto make_unary_operator() {
		return boost::phoenix::bind(
			[](auto const child) {return unary_operator<Op>{child}; }
		, boost::spirit::qi::_1);
	}
	auto make_tuple() {
		return boost::phoenix::bind(
			[](const auto tuple_impl) {
			return tuple_impl.is_initialized() ? tuple_impl.get() : std::vector<expression>{};
		}
		, boost::spirit::qi::_1);
	}


	auto make_function() {
		return boost::phoenix::bind(
			[](auto const name, const auto args) {
			return function{ name,  args };
		}
			, boost::spirit::qi::_1
			, boost::spirit::qi::_2);
	}


	auto make_variable() {
		return boost::phoenix::bind(
			[](const auto name) {
			return variable{ name};
		}
		, boost::spirit::qi::_1);
	}


	auto make_arrow() {
		return boost::phoenix::bind(
			[](auto const lhs, auto const rhs) {
			return arrow{lhs, rhs}; }
		, boost::spirit::qi::_val, boost::spirit::qi::_1);
	}





	namespace Myparser {
		using namespace boost::spirit;
		//using Iterator = decltype(std::declval<std::string>().begin());
		template <class Iterator>

		struct expr_grammar :public qi::grammar<Iterator, expression(), ascii::space_type> {
			qi::rule<Iterator, expression(), ascii::space_type> expr, eq, and, rel, additive, multiple,arrow;
			qi::rule<Iterator, std::vector<expression>(), ascii::space_type> tuple;
			qi::rule<Iterator, std::vector<expression>(), ascii::space_type> tuple_impl;
			qi::rule<Iterator, std::string(), ascii::space_type> code,quoted;
			qi::rule<Iterator, function(), ascii::space_type> func;
			qi::rule<Iterator, variable(), ascii::space_type> var;

			std::string charset = "a-zA-Z";
			std::string numset = "0-9";

			expr_grammar() :expr_grammar::base_type(expr) {
				code = lexeme[qi::char_(charset) >> *(qi::char_(charset + numset))];

				quoted %= lexeme['"' >> *(qi::char_ - '"') >> '"'];

				expr = eq[_val = _1] >> *((qi::lit("==") >> eq[_val = make_binray_operator<operators::eq>()])
					| (qi::lit("!=") >> eq[_val = make_binray_operator<operators::noteq>()]));

				eq = and[_val = _1] >> *((qi::lit("&&") >> and[_val = make_binray_operator<operators::and>()])
					| (qi::lit("||") >> and[_val = make_binray_operator<operators:: or >()]));


				and = additive[_val = _1] >> *((qi::lit("+") >> additive[_val = make_binray_operator<operators::add>()])
					| (qi::lit("-") >> additive[_val = make_binray_operator<operators::sub>()]));

				additive = multiple[_val = _1] >> *((qi::lit("*") >> multiple[_val = make_binray_operator<operators::mul>()])
					| (qi::lit("/") >> multiple[_val = make_binray_operator<operators::div>()]));

				multiple = arrow[_val = _1] >> *(qi::lit("->") >> (func | var)[_val = make_arrow()]);

				arrow = double_[_val = _1]
					| lit("(") >> expr[_val = _1] >> lit(")")
					| func[_val = _1]
					| var[_val = _1]
					| quoted[_val = _1];

				func = (code >> lit("(") >> tuple >> lit(")"))[_val = make_function()];

				var = code[_val = make_variable()];

				tuple = (-tuple_impl)[_val = make_tuple()];

				tuple_impl = (expr % lit(","))[_val = _1];
			}
		};

	}
}



namespace MyParser {
	expression parse_impl(const std::string& s) {
		Myparser::expr_grammar<decltype(s.begin())> exp;
		expression tree;
		auto b = s.begin();
		auto e = s.end();
		boost::spirit::qi::phrase_parse(b, e, exp, boost::spirit::ascii::space, tree);
		return tree;
	}

	expression parse_impl_debug(const std::string& s) {
		Myparser::expr_grammar<decltype(s.begin())> exp;
		expression tree;
		auto b = s.begin();
		auto e = s.end();
		boost::spirit::qi::phrase_parse(b, e, exp, boost::spirit::ascii::space, tree);
		std::string str(b,e);
		std::cout << "parsing stoped here:" << str << std::endl;
		return tree;
	}
}