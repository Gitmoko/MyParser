#include <iostream>
#include <string>
#include <cmath>
#include <map>
#include <vector>
#include <sstream>

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3 

#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


#include"MyParserSyntax_Impl.h"



namespace MyParser {
	expression parse_impl(const std::string& s) {
		MyParser::expr_grammar<decltype(s.begin())> exp;
		expression tree;
		auto b = s.begin();
		auto e = s.end();
		boost::spirit::qi::phrase_parse(b, e, exp, boost::spirit::ascii::space, tree);
		return tree;
	}

	expression parse_impl_debug(const std::string& s) {
		MyParser::expr_grammar<decltype(s.begin())> exp;
		expression tree;
		auto b = s.begin();
		auto e = s.end();
		boost::spirit::qi::phrase_parse(b, e, exp, boost::spirit::ascii::space, tree);
		if (b != e) {
			auto err = compile_failed{};
			err.what = exp.errorwhat.str();
			err.pos = exp.errorpos.str();
			throw err;
		}
		return tree;
	}
}