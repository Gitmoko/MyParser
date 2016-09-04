#pragma once
#include"MyParser_Boost_Cfg.h"
#include"MyParser_Binop.h"

namespace MyParser {
	struct visitor_decode : public boost::static_visitor<std::string> {

		std::string operator()(const double & constant)const {
			return std::to_string(constant);
		}

		std::string operator()(const std::string & str)const {
			return{ str };
		}

		template< MyParser::operators Op>
		std::string operator() (const binary_operator < Op >& op)const {
			using calc = MyParser::Calculaters<Op>;
			std::string&& l = boost::apply_visitor(*this,op.left);
			std::string&& r = boost::apply_visitor(*this, op.right);
			return "(" + l + ")" + calc::Symbol() + "(" + r + ")";
		}

		template<MyParser::unary_operators Op>
		std::string operator()(const unary_operator <Op>& op)const {
			using calc = MyParser::UnaryCalculaters<Op>;
			std::string val = boost::apply_visitor(*this, op.child);
			return calc::Symbol() + "(" + val + ")";
		}


		std::string operator() (const variable & op)const {
			return op.name;
		}

		std::string operator() (const function & op)const {
			std::string ret;
			ret += op.name + "(";
			auto&  list = op.args;
			for (auto beg = list.begin(),it = beg,end = list.end();it != end;++it) {
				ret += boost::apply_visitor(*this, *it);
				if ((it+1) != end) {
					ret += ",";
				}
			}
			ret += ")";
			return ret;
		}

		std::string operator()(const scope_operator<function> & scope)const {
			std::string ret;
			ret += scope.scope_name + "::";
			ret += (*this)(scope.f);
			return ret;
		}

		std::string operator()(const scope_operator<variable> &scope)const {
			std::string ret;
			ret += scope.scope_name + "::";
			ret += (*this)(scope.v);
			return ret;
		}


		std::string operator() (const arrow & op)const {
			std::string ret;
			ret += boost::apply_visitor(*this, op.l);
			ret += "->";
			ret += boost::apply_visitor(*this, op.r);
			return ret;
		}

		std::string operator() (const tuple & op)const {
			std::string ret;
			ret += "{";
			auto&  list = op.elems;
			for (auto beg = list.begin(), it = beg, end = list.end(); it != end; ++it) {
				ret += boost::apply_visitor(*this, *it);
				if (std::distance(beg, it) != list.size() - 1) {
					ret += ",";
				}
			}
			ret += "}";
			return ret;
		}

		template<class Expr>
		static std::string get( const Expr& arg) {
			return boost::apply_visitor(visitor_decode{}, arg);
		}

	};
}