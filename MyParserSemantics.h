#ifndef MyParserFrontH
#define MyParserFrontH

#include <string>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/get.hpp>
#include<vector>
#include<memory>
#include<functional>
#include<iostream>
#include "MyParserSyntax.h"

namespace MyParser {

	struct bad_operand {};

	template<class... Instance>
	struct v_tuple;

	template<class... T>
	struct Instance {
		boost::variant<T...> instance;
		Instance(boost::variant<T...> i_) :instance(i_) {}
	};

	template<class... T>
	using return_t = boost::variant<
		double
		, std::string
		, Instance<T...>
		, boost::recursive_wrapper<v_tuple<T...>>>;

	template<class... T>
	struct v_tuple {
		using mem_t = std::vector<return_t<T...>>;
		mem_t tuple;
		v_tuple(const mem_t& arg) :tuple(arg) { }
	};


	template<class Visitor_v, class Visitor_f, class... T>
	struct visitor : public boost::static_visitor<return_t<T...>> {
		using Instance_type = Instance<T...>;
		using tuple_type = v_tuple<T...>;
		using return_type = return_t<T...>;

		const Instance<T...>& i;
		visitor(const Instance<T...>& i_) :i(i_) {}

		return_t<T...> operator()(const double & constant)const {
			return{ constant };
		}

		return_t<T...> operator()(const std::string & str)const {
			return{ str };
		}


		return_t<T...> operator()(const binary_operator < operators::eq >& op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l == *r };
		}
		return_t<T...> operator()(const binary_operator < operators::noteq > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l != *r };
		}
		return_t<T...> operator()(const binary_operator < operators::and > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l&&*r };
		}
		return_t<T...> operator()(const binary_operator < operators:: or > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l || *r };
		}
		return_t<T...> operator()(const binary_operator < operators::relless > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l < *r };
		}
		return_t<T...> operator()(const binary_operator < operators::relmore > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l > *r };
		}
		return_t<T...> operator()(const binary_operator < operators::releqless > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l <= *r };
		}
		return_t<T...> operator()(const binary_operator < operators::releqmore > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l >= *r };
		}
		return_t<T...> operator()(const binary_operator < operators::add > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			else {
				return{ *l + *r };
			}
		}
		return_t<T...> operator()(const binary_operator < operators::sub > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l - *r };
		}
		return_t<T...> operator()(const binary_operator < operators::mul > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l**r };
		}
		return_t<T...> operator()(const binary_operator < operators::div > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l / *r };
		}

		return_t<T...> operator()(const variable & op)const {
			Visitor_v v(op.name);
			auto ret = boost::apply_visitor(v, i.instance);
			return ret;
		}

		return_t<T...> operator()(const function & op)const {
			std::vector<return_t<T...>> args;
			auto&  list = op.args;
			for (auto& elem : list) {
				args.push_back(boost::apply_visitor(*this, elem));
			}
			Visitor_f v(op.name, v_tuple<T...>{ args });
			auto ret = boost::apply_visitor(v, i.instance);//calc function
			return ret;
		}

		return_t<T...> operator()(const arrow & op)const {
			auto left = boost::apply_visitor(*this, op.l);
			auto ref = boost::get<Instance<T...>>(&left);
			if (ref) {
				auto ret = get(*ref, op.r);
				return ret;
			}
			else {
				throw MyParser::bad_operand{};
			}
		}

		template<class Expr>
		static return_t<T...> get(const Instance<T...>& i, const Expr& arg) {
			auto v = visitor<Visitor_v, Visitor_f, T...>{ i };
			return boost::apply_visitor(v, arg);
		}

	};

	template<class Visitor_v, class Visitor_f, class... T>
	struct visitor_debug : public boost::static_visitor<return_t<T...>> {
		using Instance_type = Instance<T...>;
		using tuple_type = v_tuple<T...>;
		using return_type = return_t<T...>;

		const Instance<T...>& i;
		visitor_debug(const Instance<T...>& i_) :i(i_) {}

		return_t<T...> operator()(const double & constant)const {
			std::cout << "constant" << std::endl;
			return{ constant };
		}

		return_t<T...> operator()(const std::string & str)const {
			std::cout << "string" << std::endl;
			return{ str };
		}


		return_t<T...> operator() (const binary_operator < operators::eq >& op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "eq" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l == *r };
		}
		return_t<T...> operator() (const binary_operator < operators::noteq > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "noteq" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l != *r };
		}
		return_t<T...> operator() (const binary_operator < operators::and > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "and" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l&&*r };
		}
		return_t<T...> operator() (const binary_operator < operators:: or > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "or" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l || *r };
		}
		return_t<T...> operator() (const binary_operator < operators::relless > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "relless" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l < *r };
		}
		return_t<T...> operator() (const binary_operator < operators::relmore > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "relmore" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l > *r };
		}
		return_t<T...> operator() (const binary_operator < operators::releqless > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "releqless" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l <= *r };
		}
		return_t<T...> operator() (const binary_operator < operators::releqmore > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "noteqmore" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l >= *r };
		}
		return_t<T...> operator() (const binary_operator < operators::add > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "add" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			else {
				return{ *l + *r };
			}
		}
		return_t<T...> operator() (const binary_operator < operators::sub > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "sub" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l - *r };
		}
		return_t<T...> operator() (const binary_operator < operators::mul > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "mul" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l**r };
		}
		return_t<T...> operator() (const binary_operator < operators::div > & op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "div" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l / *r };
		}

		return_t<T...> operator() (const variable & op)const {
			std::cout << "variable" << std::endl;
			Visitor_v v(op.name);
			auto ret = boost::apply_visitor(v, i.instance);
			return ret;
		}

		return_t<T...> operator() (const function & op)const {
			std::cout << "function" << std::endl;
			std::vector<return_t<T...>> args;
			const auto&  list = op.args;
			for (auto& elem : list) {
				args.push_back(boost::apply_visitor(*this, elem));
			}
			Visitor_f v(op.name, v_tuple<T...>{ args });
			auto ret = boost::apply_visitor(v, i.instance);//calc function
			return ret;
		}

		return_t<T...> operator() (const arrow & op)const {
			std::cout << "arrow" << std::endl;
			auto left = boost::apply_visitor(*this, op.l);
			auto ref = boost::get<Instance<T...>>(&left);
			if (ref) {
				auto ret = get(*ref, op.r);
				return ret;
			}
			else {
				throw MyParser::bad_operand{};
			}
		}

		template<class Expr>
		static return_t<T...> get(const Instance<T...>& i, const Expr& arg) {
			auto v = visitor_debug<Visitor_v, Visitor_f, T...>{ i };
			return boost::apply_visitor(v, arg);
		}

	};



}



#endif