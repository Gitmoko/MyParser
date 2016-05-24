#ifndef MyParserParseH
#define MyParserParseH
#include<string>
#include <string>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/get.hpp>
#include<vector>
#include<memory>
#include<functional>

namespace MyParser {
	enum struct operators {
		eq, noteq
		, and, or
		, relless, relmore, releqless, releqmore
		, add, sub
		, mul, div
	};

	template<operators Op>
	struct binary_operator;

	template<operators Op>
	struct unary_operator;


	struct variable {
		std::string name;
		variable(const std::string& name) :name(name) {}
		variable() {}
	};


	struct function;

	struct arrow;

	using expression = boost::variant<double
		, std::string
		, boost::recursive_wrapper<binary_operator<operators::eq>>
		, boost::recursive_wrapper<binary_operator<operators::noteq>>
		, boost::recursive_wrapper<binary_operator<operators::and>>
		, boost::recursive_wrapper<binary_operator<operators:: or >>
		, boost::recursive_wrapper<binary_operator<operators::relless>>
		, boost::recursive_wrapper<binary_operator<operators::relmore>>
		, boost::recursive_wrapper<binary_operator<operators::releqless>>
		, boost::recursive_wrapper<binary_operator<operators::releqmore>>
		, boost::recursive_wrapper<binary_operator<operators::add>>
		, boost::recursive_wrapper<binary_operator<operators::sub>>
		, boost::recursive_wrapper<binary_operator<operators::mul>>
		, boost::recursive_wrapper<binary_operator<operators::div>>
		, boost::recursive_wrapper<function>
		, boost::recursive_wrapper<arrow>
		, variable
	>;

	template<operators Op>
	struct binary_operator {
		expression left;
		expression right;
		binary_operator(expression const& lhs, expression const & rhs) :left{ std::move(lhs) }, right{ std::move(rhs) } {}
	};

	template<operators Op>
	struct unary_operator {
		expression child;
		unary_operator(expression const& child) :child{ std::move(child) } {}
	};

	struct function {
		using arg_t = std::vector<expression>;
		arg_t args;
		std::string name;
		function(const std::string& name, const arg_t & a) :name(name), args(std::move(a)) {}
		function() {}
	};

	struct arrow {
		expression l;
		boost::variant<variable, function> r;
		arrow(const expression& _l, const boost::variant<variable, function>& _r) :l(std::move(_l)), r(std::move(_r)) {}
	};
}

#endif