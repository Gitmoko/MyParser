#ifndef MyParserParseH
#define MyParserParseH

#ifdef CNSPARSER_DLLUSE
#ifdef CNSPARSER_EXPORTS
#define MYPARSER_API __declspec(dllexport) 
#else
#define MYPARSER_API __declspec(dllimport) 
#endif
#else
#define MYPARSER_API 
#endif


#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS 
#define BOOST_MPL_LIMIT_LIST_SIZE 30
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30

#include<string>
#include <string>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include<vector>
#include<memory>
#include<functional>


namespace MyParser {
	enum struct operators {
		eq, noteq
		, and_, or_
		, relless, relmore, releqless, releqmore
		, add, sub
		, mul, div
	};

	enum struct unary_operators {
		not_,plus,minus
	};

	template<operators Op>
	struct binary_operator;

	template<unary_operators Op>
	struct unary_operator;

	template<class T>
	struct scope_operator;


	struct variable {
		std::string name;
		variable(std::string name) :name(name) {}
		variable() {}
	};


	struct function;

	struct arrow;

	struct tuple;

	using expression = boost::variant<double
		, std::string
		, boost::recursive_wrapper<binary_operator<operators::eq>>
		, boost::recursive_wrapper<binary_operator<operators::noteq>>
		, boost::recursive_wrapper<binary_operator<operators::and_>>
		, boost::recursive_wrapper<binary_operator<operators:: or_ >>
		, boost::recursive_wrapper<binary_operator<operators::relless>>
		, boost::recursive_wrapper<binary_operator<operators::relmore>>
		, boost::recursive_wrapper<binary_operator<operators::releqless>>
		, boost::recursive_wrapper<binary_operator<operators::releqmore>>
		, boost::recursive_wrapper<binary_operator<operators::add>>
		, boost::recursive_wrapper<binary_operator<operators::sub>>
		, boost::recursive_wrapper<binary_operator<operators::mul>>
		, boost::recursive_wrapper<binary_operator<operators::div>>
		, boost::recursive_wrapper<unary_operator<unary_operators::plus>>
		, boost::recursive_wrapper<unary_operator<unary_operators::minus>>
		, boost::recursive_wrapper<unary_operator<unary_operators::not_>>
		, boost::recursive_wrapper<scope_operator<function>>
		, boost::recursive_wrapper<scope_operator<variable>>
		, boost::recursive_wrapper<function>
		, boost::recursive_wrapper<arrow>
		, boost::recursive_wrapper<tuple>
		, variable
	>;

	template<operators Op>
	struct binary_operator {
		expression left;
		expression right;
		binary_operator(expression const lhs, expression const  rhs) :left{ lhs }, right{ rhs } {}
	};

	template<unary_operators Op>
	struct unary_operator {
		expression child;
		unary_operator(expression const child) :child{ child } {}
	};

	struct function {
		using arg_t = std::vector<expression>;
		arg_t args;
		std::string name;
		function(const std::string name, const arg_t  a) :name(name), args(a) {}
		function() {}
	};

	template <>
	struct scope_operator<function>{
		function f;
		std::string scope_name;
		scope_operator(std::string str, function f_) :f(f_), scope_name(str) {}
	};

	template <>
	struct scope_operator<variable> {
		variable v;
		std::string scope_name;
		scope_operator(std::string str, variable v_) :v(v_), scope_name(str) {}
	};

	

	struct arrow {
		expression l;
		boost::variant<variable, function> r;
		arrow(const expression _l, const boost::variant<variable, function> _r) :l(_l), r(_r) {}
	};

	struct tuple {
		using elem_t = std::vector<expression>;
		elem_t elems;
		tuple(const elem_t  a) : elems(a) {}
		tuple() {}

	};


	expression parse_impl(const std::string& s);
	expression parse_impl_debug(const std::string& s);

	struct compile_failed { 
		std::string what;
		std::string pos;
	};

}

#endif