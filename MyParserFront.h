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
#include "MyParserParse.h"

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

		Instance<T...>& i;
		visitor(Instance<T...>& i_) :i(i_) {}

		return_t<T...> operator()(double & constant) {
			return{ constant };
		}

		return_t<T...> operator()(std::string & str) {
			return{ str };
		}


		return_t<T...> operator() (binary_operator < operators::eq >& op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l == *r };
		}
		return_t<T...> operator() (binary_operator < operators::noteq > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l != *r };
		}
		return_t<T...> operator() (binary_operator < operators::and > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l&&*r };
		}
		return_t<T...> operator() (binary_operator < operators:: or > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l || *r };
		}
		return_t<T...> operator() (binary_operator < operators::relless > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l < *r };
		}
		return_t<T...> operator() (binary_operator < operators::relmore > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l > *r };
		}
		return_t<T...> operator() (binary_operator < operators::releqless > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l <= *r };
		}
		return_t<T...> operator() (binary_operator < operators::releqmore > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l >= *r };
		}
		return_t<T...> operator() (binary_operator < operators::add > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			else {
				return{ *l + *r };
			}
		}
		return_t<T...> operator() (binary_operator < operators::sub > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l - *r };
		}
		return_t<T...> operator() (binary_operator < operators::mul > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l**r };
		}
		return_t<T...> operator() (binary_operator < operators::div > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l / *r };
		}

		return_t<T...> operator() (variable & op) {
			Visitor_v v(op.name);
			auto ret = boost::apply_visitor(v, i.instance);
			return ret;
		}

		return_t<T...> operator() (function & op) {
			std::vector<return_t<T...>> args;
			auto&  list = op.args;
			for (auto& elem : list) {
				args.push_back(boost::apply_visitor(*this, elem));
			}
			Visitor_f v(op.name, v_tuple<T...>{ args });
			auto ret = boost::apply_visitor(v, i.instance);//calc function
			return ret;
		}

		return_t<T...> operator() (arrow & op) {
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
		static return_t<T...> get(Instance<T...>& i, Expr& arg) {
			auto v = visitor<Visitor_v, Visitor_f, T...>{ i };
			return boost::apply_visitor(v, arg);
		}

	};

	template<class Visitor_v, class Visitor_f, class... T>
	struct visitor_debug : public boost::static_visitor<return_t<T...>> {
		using Instance_type = Instance<T...>;
		using tuple_type = v_tuple<T...>;
		using return_type = return_t<T...>;

		Instance<T...>& i;
		visitor_debug(Instance<T...>& i_) :i(i_) {}

		return_t<T...> operator()(double & constant) {
			std::cout << "constant" << std::endl;
			return{ constant };
		}

		return_t<T...> operator()(std::string & str) {
			std::cout << "string" << std::endl;
			return{ str };
		}


		return_t<T...> operator() (binary_operator < operators::eq >& op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "eq" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l == *r };
		}
		return_t<T...> operator() (binary_operator < operators::noteq > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "noteq" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l != *r };
		}
		return_t<T...> operator() (binary_operator < operators::and > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "and" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l&&*r };
		}
		return_t<T...> operator() (binary_operator < operators:: or > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "or" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l || *r };
		}
		return_t<T...> operator() (binary_operator < operators::relless > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "relless" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l < *r };
		}
		return_t<T...> operator() (binary_operator < operators::relmore > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "relmore" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l > *r };
		}
		return_t<T...> operator() (binary_operator < operators::releqless > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "releqless" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l <= *r };
		}
		return_t<T...> operator() (binary_operator < operators::releqmore > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "noteqmore" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l >= *r };
		}
		return_t<T...> operator() (binary_operator < operators::add > & op) {
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
		return_t<T...> operator() (binary_operator < operators::sub > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "sub" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l - *r };
		}
		return_t<T...> operator() (binary_operator < operators::mul > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "mul" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l**r };
		}
		return_t<T...> operator() (binary_operator < operators::div > & op) {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			std::cout << "div" << std::endl;
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ *l / *r };
		}

		return_t<T...> operator() (variable & op) {
			std::cout << "variable" << std::endl;
			Visitor_v v(op.name);
			auto ret = boost::apply_visitor(v, i.instance);
			return ret;
		}

		return_t<T...> operator() (function & op) {
			std::cout << "function" << std::endl;
			std::vector<return_t<T...>> args;
			auto&  list = op.args;
			for (auto& elem : list) {
				args.push_back(boost::apply_visitor(*this, elem));
			}
			Visitor_f v(op.name, v_tuple<T...>{ args });
			auto ret = boost::apply_visitor(v, i.instance);//calc function
			return ret;
		}

		return_t<T...> operator() (arrow & op) {
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
		static return_t<T...> get(Instance<T...>& i, Expr& arg) {
			auto v = visitor_debug<Visitor_v, Visitor_f, T...>{ i };
			return boost::apply_visitor(v, arg);
		}

	};

	expression parse_impl(const std::string& s);

	template<class Visitor_v, class Visitor_f, class... T>
	return_t<T...> Parse(const std::string& s, boost::variant<T...> i) {
		auto tree = parse_impl(s);
		if (!tree.empty()) {
			auto ret = visitor<Visitor_v, Visitor_f, T...>::get(Instance<T...>{i}, tree);
			return ret;
		}
		return{};
	}

	template<class Visitor_v, class Visitor_f, class... T>
	return_t<T...> Parse_debug(const std::string& s, boost::variant<T...> i) {
		auto tree = parse_impl(s);
		if (!tree.empty()) {
			auto ret = visitor_debug<Visitor_v, Visitor_f, T...>::get(Instance<T...>{i}, tree);
			return ret;
		}
		return{};

	}

	template<class Visitor_v,class Visitor_f,class ...T>
	struct Parser_api {
	public:

		using v_tuple_type = v_tuple<T...>;
		using return_t_type = return_t<T...>;
		using Instance_type = Instance<T...>;
	public:

		static return_t<T...> Parse(const std::string& s, boost::variant<T...> i) {
			auto tree = parse_impl(s);
			if (!tree.empty()) {
				auto ret = visitor<Visitor_v, Visitor_f, T...>::get(Instance<T...>{i}, tree);
				return ret;
			}
			return{};
		}
		static return_t<T...> Parse_debug(const std::string& s, boost::variant<T...> i) {
			auto tree = parse_impl(s);
			if (!tree.empty()) {
				auto ret = visitor_debug<Visitor_v, Visitor_f, T...>::get(Instance<T...>{i}, tree);
				return ret;
			}
			return{};

		}
	};

}



#endif