#ifndef MyParser_Parse_ImplH
#define MyParser_Parse_ImplH


#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include"MyParserSyntax.h"

namespace MyParser {

	namespace {
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
				return variable{ name };
			}
			, boost::spirit::qi::_1);
		}


		auto make_arrow() {
			return boost::phoenix::bind(
				[](auto const lhs, auto const rhs) {
				return arrow{ lhs, rhs }; }
			, boost::spirit::qi::_val, boost::spirit::qi::_1);
		}
	}


	using namespace boost::spirit;
	//using Iterator = decltype(std::declval<std::string>().begin());
	template <class Iterator>
	struct expr_grammar :public qi::grammar<Iterator, expression(), ascii::space_type> {
	private:
		qi::rule<Iterator, expression(), ascii::space_type> expr, eq, and, rel, additive, multiple, arrow;
		qi::rule<Iterator, std::vector<expression>(), ascii::space_type> tuple;
		qi::rule<Iterator, std::vector<expression>(), ascii::space_type> tuple_impl;
		qi::rule<Iterator, std::string(), ascii::space_type> code, quoted;
		qi::rule<Iterator, function(), ascii::space_type> func;
		qi::rule<Iterator, variable(), ascii::space_type> var;

		std::string charset = "a-zA-Z";
		std::string numset = "0-9";
	public:
		expr_grammar() :expr_grammar::base_type(expr, "expr") {
			code = lexeme[qi::char_(charset) > *(qi::char_(charset + numset))];

			quoted %= lexeme['"' > *(qi::char_ - '"') > '"'];

			expr = rel[_val = _1] >> *((qi::lit("==") > rel[_val = make_binray_operator<operators::eq>()])
				| (qi::lit("!=") > rel[_val = make_binray_operator<operators::noteq>()]));

			rel = eq[_val = _1] >> *((qi::lit("<") > eq[_val = make_binray_operator<operators::relless>()])
				| (qi::lit(">") > eq[_val = make_binray_operator<operators::relmore>()])
				| (qi::lit("<=") > eq[_val = make_binray_operator<operators::releqless>()])
				| (qi::lit(">=") > eq[_val = make_binray_operator<operators::releqmore>()]));

			eq = and[_val = _1] >> *((qi::lit("&&") > and[_val = make_binray_operator<operators::and>()])
				| (qi::lit("||") > and[_val = make_binray_operator<operators:: or >()]));


			and = additive[_val = _1] >> *((qi::lit("+") > additive[_val = make_binray_operator<operators::add>()])
				| (qi::lit("-") > additive[_val = make_binray_operator<operators::sub>()]));

			additive = multiple[_val = _1] >> *((qi::lit("*") > multiple[_val = make_binray_operator<operators::mul>()])
				| (qi::lit("/") > multiple[_val = make_binray_operator<operators::div>()]));

			multiple = arrow[_val = _1] >> *(qi::lit("->") > (func | var)[_val = make_arrow()]);

			arrow = double_[_val = _1]
				| lit("(") > expr[_val = _1] > lit(")")
				| func[_val = _1]
				| var[_val = _1]
				| quoted[_val = _1];

			func = (code >> lit("(") > tuple > lit(")"))[_val = make_function()];

			var = code[_val = make_variable()];

			tuple = (-tuple_impl)[_val = make_tuple()];

			tuple_impl = (expr % lit(","))[_val = _1];

			expr.name("expr");
			eq.name("eq");
			and.name("and");
			rel.name("rel");
			additive.name("additive");
			multiple.name("multiple");
			arrow.name("arrow");
			tuple.name("tuple");
			tuple_impl.name("tuple_impl");
			code.name("code");
			quoted.name("quoted");
			func.name("func");
			var.name("var");


			qi::on_error<qi::fail>
				(
					expr
					, boost::phoenix::if_(boost::phoenix::ref(errorwhat) && boost::phoenix::ref(errorpos))[
						errorwhat << qi::_4,
							errorpos << boost::phoenix::construct<std::string>(qi::_3, qi::_2)
					]
					);

		}

		std::ostringstream errorwhat;
		std::ostringstream errorpos;

	};


}
#endif