#ifndef MyParser_Parse_ImplH
#define MyParser_Parse_ImplH

#include"MyParserSyntax.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
namespace MyParser {

	namespace {
		//constexpr auto  Op = operators::sub;
		template<operators Op>
		auto make_binray_operator() {
			return boost::phoenix::bind(
				[](auto const lhs, auto const rhs) {return binary_operator<Op>{lhs, rhs}; }
			, boost::spirit::qi::_val, boost::spirit::qi::_1);
		}

		template<unary_operators Op>
		auto make_unary_operator() {
			return boost::phoenix::bind(
				[](auto const child) {return unary_operator<Op>{child}; }
			, boost::spirit::qi::_1);
		}
		auto make_args() {
			return boost::phoenix::bind(
				//optional_arg is optional<std::vector<expression>>
				[](const auto optional_arg) {
				return optional_arg.is_initialized() ? optional_arg.get() : std::vector<expression>{};
			}
			, boost::spirit::qi::_1);
		}

		auto make_tuple() {
			return boost::phoenix::bind(
				[](const auto argtuple) {
				tuple ret(argtuple);
				return ret;
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

		auto make_scope() {
			return boost::phoenix::bind(
				[](const boost::optional<std::string> name, const boost::variant<function,variable> op) {
					std::string str = name.is_initialized() ? name.get() : "";
					if (op.which() == 0) {
						return expression{ scope_operator<function>{ str,boost::get<function>(op) } };
					}
					else if(op.which() == 1){
						return expression{ scope_operator<variable>{ str,boost::get<variable>(op) } };
					}
					else {
						throw 0;
					}
				 }
			, boost::spirit::qi::_1, boost::spirit::qi::_2);
		}

	}


	using namespace boost::spirit;
	//using Iterator = decltype(std::declval<std::string>().begin());
	template <class Iterator>
	struct expr_grammar :public qi::grammar<Iterator, expression(), ascii::space_type> {
	private:
		qi::rule<Iterator, expression(), ascii::space_type> wholeexpr;
		qi::rule<Iterator, expression(), ascii::space_type> expr, eq, and, rel, additive, multiple, arrow;
		qi::rule<Iterator, std::vector<expression>(), ascii::space_type> args;
		qi::rule<Iterator, std::vector<expression>(), ascii::space_type> args_impl;
		qi::rule<Iterator, tuple(), ascii::space_type> tpl;
		qi::rule<Iterator, std::string(), ascii::space_type> code, quoted;
		qi::rule<Iterator, function(), ascii::space_type> func;
		qi::rule<Iterator, variable(), ascii::space_type> var;
		qi::rule<Iterator, expression(), ascii::space_type> unary;
		qi::rule<Iterator, expression(), ascii::space_type> scope;

		std::string charset = "a-zA-Z";
		std::string numset = "0-9";
	public:
		expr_grammar() :expr_grammar::base_type(wholeexpr, "wholeexpr") {
			code = lexeme[qi::char_(charset) > *(qi::char_(charset + numset))];

			quoted %= lexeme['"' > *(qi::char_ - '"') > '"'];

			wholeexpr = expr[_val = _1] > qi::eoi;

			expr = rel[_val = _1] >> *((qi::lit("==") > rel[_val = make_binray_operator<operators::eq>()])
				| (qi::lit("!=") > rel[_val = make_binray_operator<operators::noteq>()]));

			rel = eq[_val = _1] >> *((qi::lit("<") > eq[_val = make_binray_operator<operators::relless>()])
				| (qi::lit(">") > eq[_val = make_binray_operator<operators::relmore>()])
				| (qi::lit("<=") > eq[_val = make_binray_operator<operators::releqless>()])
				| (qi::lit(">=") > eq[_val = make_binray_operator<operators::releqmore>()]));

			eq = and[_val = _1] >> *((qi::lit("&&") > and[_val = make_binray_operator<operators::and_>()])
				| (qi::lit("||") > and[_val = make_binray_operator<operators::or_ >()]));


			and = additive[_val = _1] >> *((qi::lit("+") > additive[_val = make_binray_operator<operators::add>()])
				| (qi::lit("-") > additive[_val = make_binray_operator<operators::sub>()]));

			additive = multiple[_val = _1] >> *((qi::lit("*") > multiple[_val = make_binray_operator<operators::mul>()])
				| (qi::lit("/") > multiple[_val = make_binray_operator<operators::div>()]));

			multiple = unary[_val = _1] >> *(qi::lit("->") > (func | var)[_val = make_arrow()]);

			unary = ((qi::lit("+")) > arrow[_val = make_unary_operator<unary_operators::plus>()])
				| ((qi::lit("-")) > arrow[_val = make_unary_operator<unary_operators::minus>()])
				| ((qi::lit("!")) > arrow[_val = make_unary_operator<unary_operators::not_>()])
				| arrow[_val = _1];

			arrow = double_[_val = _1]
				| lit("(") > expr[_val = _1] > lit(")")
				| lit("{") > tpl[_val = _1] > lit("}")
				| scope[_val = _1]
				| quoted[_val = _1];

			scope = ((-code) >> lit("::") > (func | var))[_val = make_scope()]
				| func[_val = _1]
				| var[_val = _1];

			func = (code >> lit("(") > args > lit(")"))[_val = make_function()];

			var = code[_val = make_variable()];

			args = (-args_impl)[_val = make_args()];

			args_impl = (expr % lit(","))[_val = _1];

			tpl = args[_val = make_tuple()];

			wholeexpr.name("wholeexpr");
			expr.name("expr");
			eq.name("eq");
			and.name("and");
			rel.name("rel");
			additive.name("additive");
			multiple.name("multiple");
			arrow.name("arrow");
			args.name("args");
			args_impl.name("args_impl");
			code.name("code");
			quoted.name("quoted");
			func.name("func");
			var.name("var");
			tpl.name("tuple");
			unary.name("unary");
			scope.name("scope");


			qi::on_error<qi::fail>
				(
					wholeexpr
					, boost::phoenix::if_(boost::phoenix::ref(errorwhat) && boost::phoenix::ref(errorpos))[
						errorwhat << qi::_4,
							(boost::phoenix::if_(boost::phoenix::ref(failflag) == false)[(boost::phoenix::ref(errorpos) << boost::phoenix::construct<std::string>(qi::_3, qi::_2)),boost::phoenix::ref(failflag) = true])
					]
					);

		}
	private:
		bool failflag = false;
	public:

		std::ostringstream errorwhat;
		std::ostringstream errorpos;

		bool isfailed() { return faliflag; }

	};

	inline expression parse_impl(const std::string& s) {
		MyParser::expr_grammar<decltype(s.begin())> exp;
		expression tree;
		auto b = s.begin();
		auto e = s.end();
		boost::spirit::qi::phrase_parse(b, e, exp, boost::spirit::ascii::space, tree);
		return tree;
	}

	inline expression parse_impl_debug(const std::string& s) {
		MyParser::expr_grammar<decltype(s.begin())> exp;
		expression tree;
		auto b = s.begin();
		auto e = s.end();
		boost::spirit::qi::phrase_parse(b, e, exp, boost::spirit::ascii::space, tree);
		if (b != e) {
			auto err = compile_failed{};
			err.what = exp.errorwhat.str();
			err.pos = exp.errorpos.str() + "[end]";
			throw err;
		}
		return tree;
	}

}
#endif