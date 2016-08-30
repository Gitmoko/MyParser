#include <iostream>
#include <string>
#include <cmath>
#include <map>
#include <vector>
#include <sstream>

#include"MyParserSyntax_Impl.h"

#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

MyParser::expression parse_impl(const std::string& s) {
	return MyParser::parse_impl(s);
}

