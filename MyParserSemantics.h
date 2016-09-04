#ifndef MyParserFrontH
#define MyParserFrontH


#include "MyParser_Boost_Cfg.h"
#include"MyParser_Binop.h"
#include "MyParserSyntax.h"
#include <string>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/get.hpp>
#include<vector>
#include<memory>
#include<functional>
#include<iostream>
#include<unordered_map>
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
		v_tuple() {}
		v_tuple(const mem_t& arg) :tuple(arg) { }
	};


	template<class Visitor_v, class Visitor_f, class... T>
	struct visitor : public boost::static_visitor<return_t<T...>> {
		using Instance_type = Instance<T...>;
		using tuple_type = v_tuple<T...>;
		using return_type = return_t<T...>;

		const Instance<T...>& i;
		std::unordered_map < std::string, std::function < return_type(tuple_type arg) > > stdfunc;
		std::unordered_map<std::string, return_type> stdconstant;
		visitor(const Instance<T...>& i_) :i(i_)
			, stdfunc({ { "sin", [](tuple_type arg) {return std::sin(boost::get<double>(arg.tuple[0])); } }
					,{"cos" , [](tuple_type arg) {return std::cos(boost::get<double>(arg.tuple[0])); } }
					,{ "tan" , [](tuple_type arg) {return std::tan(boost::get<double>(arg.tuple[0])); } }
					,{ "pow" , [](tuple_type arg) {return std::pow(boost::get<double>(arg.tuple[0]),boost::get<double>(arg.tuple[1])); } }
					,{ "log" , [](tuple_type arg) {return std::log(boost::get<double>(arg.tuple[1]))/std::log(boost::get<double>(arg.tuple[0])); } }
					,{"get" , [](tuple_type arg) {return boost::get<tuple_type>(arg.tuple[1]).tuple[boost::get<double>(arg.tuple[0])]; } }
					,{ "rot2D" , [](tuple_type arg) {
									auto &vec = boost::get<tuple_type>(arg.tuple[0]).tuple;
									auto rad = boost::get<double>(arg.tuple[1]);
									auto radcos = std::cos(rad);
									auto radsin = std::sin(rad);
									auto x = boost::get<double>(vec[0]);
									auto y = boost::get<double>(vec[1]);
									return tuple_type{ tuple_type::mem_t{x*radcos - y*radsin,x*radsin + y*radcos} };
								}}
			})
			, stdconstant({ { "PI", return_type{3.1415926535897932384626} }, { "e", return_type{ 2.718281828459045} } }) {}

		return_t<T...> operator()(const double & constant)const {
			return{ constant };
		}

		return_t<T...> operator()(const std::string & str)const {
			return{ str };
		}

		template<MyParser::operators Op>
		return_t<T...> operator()(const binary_operator <Op>& op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ MyParser::Calculaters<Op>::Calc(*l,*r) };
		}

		template<MyParser::unary_operators Op>
		return_t<T...> operator()(const unary_operator <Op>& op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.child))));
			if (!l) {
				throw MyParser::bad_operand{};
			}
			return{ MyParser::UnaryCalculaters<Op>::Calc(*l) };
		}

		return_t<T...> operator()(const variable & op)const {
			auto tmpv = std::bind(Visitor_v{}, op.name, std::placeholders::_1);
			auto ret = boost::apply_visitor(tmpv, i.instance);
			return ret;
		}

		return_t<T...> operator()(const function & op)const {
			std::vector<return_t<T...>> args;
			auto&  list = op.args;
			for (auto& elem : list) {
				args.push_back(boost::apply_visitor(*this, elem));
			}
			auto tmpv = std::bind(Visitor_f{}, op.name, tuple_type{ args }, std::placeholders::_1);
			auto ret = boost::apply_visitor(tmpv, i.instance);//calc function
			return ret;
		}

		return_t<T...> operator()(const scope_operator<function> & scope)const {
			auto op = scope.f;
			std::vector<return_t<T...>> args;
			auto&  list = op.args;
			for (auto& elem : list) {
				args.push_back(boost::apply_visitor(*this, elem));
			}
			if (scope.scope_name.size() == 0) {
				return stdfunc.at(op.name)(tuple_type{ args });
			}
			auto tmpv = std::bind(Visitor_f{}, op.name, tuple_type{ args }, std::placeholders::_1);
			auto ret = boost::apply_visitor(tmpv, i.instance);//calc function
			return ret;
		}

		return_t<T...> operator()(const scope_operator<variable> &scope)const {
			auto op = scope.v;
			if (scope.scope_name.size() == 0) {
				return stdconstant.at(op.name);
			}
			auto tmpv = std::bind(Visitor_v{}, op.name, std::placeholders::_1);
			auto ret = boost::apply_visitor(tmpv, i.instance);
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

		return_t<T...> operator() (const tuple & op)const {
			v_tuple<T...> ret;
			for (auto& elem : op.elems) {
				auto value = boost::apply_visitor(*this, elem);
				ret.tuple.push_back(value);
			}
			return ret;
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

		std::unordered_map < std::string, std::function < return_type(tuple_type arg) > > stdfunc;
		std::unordered_map<std::string, return_type> stdconstant;
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

		template< MyParser::operators Op>
		return_t<T...> operator() (const binary_operator < Op >& op)const {
			using calc = MyParser::Calculaters<Op>;
			std::cout << calc::Name() << std::endl;
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.left))));
			auto r = (boost::get<double>(&(boost::apply_visitor(*this, op.right))));
			if ((!l) || (!r)) {
				throw MyParser::bad_operand{};
			}
			return{ calc::Calc(*l,*r) };
		}

		template<MyParser::unary_operators Op>
		return_t<T...> operator()(const unary_operator <Op>& op)const {
			auto l = (boost::get<double>(&(boost::apply_visitor(*this, op.child))));
			using calc = MyParser::UnaryCalculaters<Op>;
			std::cout << calc::Name() << std::endl;
			if (!l) {
				throw MyParser::bad_operand{};
			}
			return{ calc::Calc(*l) };
		}
		

		return_t<T...> operator() (const variable & op)const {
			std::cout << "variable" << std::endl; 
			auto tmpv = std::bind(Visitor_v{}, op.name, std::placeholders::_1);
			auto ret = boost::apply_visitor(tmpv, i.instance);
			return ret;
		}

		return_t<T...> operator() (const function & op)const {
			std::cout << "function" << std::endl;
			std::vector<return_t<T...>> args;
			auto&  list = op.args;
			for (auto& elem : list) {
				args.push_back(boost::apply_visitor(*this, elem));
			}
			auto tmpv = std::bind(Visitor_f{}, op.name, tuple_type{ args }, std::placeholders::_1);
			auto ret = boost::apply_visitor(tmpv, i.instance);//calc function
			return ret;
		}

		return_t<T...> operator()(const scope_operator<function> & scope)const {
			std::cout << scope.scope_name << " scoped function" << std::endl;
			auto op = scope.f;
			std::vector<return_t<T...>> args;
			auto&  list = op.args;
			for (auto& elem : list) {
				args.push_back(boost::apply_visitor(*this, elem));
			}
			if (scope.scope_name.size() == 0) {
				return stdfunc.at(op.name)(tuple_type{ args });
			}
			auto tmpv = std::bind(Visitor_f{}, op.name, tuple_type{ args }, std::placeholders::_1);
			auto ret = boost::apply_visitor(tmpv, i.instance);//calc function
			return ret;
		}

		return_t<T...> operator()(const scope_operator<variable> &scope)const {
			std::cout << scope.scope_name << " scoped variable" << std::endl;
			auto op = scope.v;
			if (scope.scope_name.size() == 0) {
				return stdconstant.at(op.name);
			}
			auto tmpv = std::bind(Visitor_v{}, op.name, std::placeholders::_1);
			auto ret = boost::apply_visitor(tmpv, i.instance);
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

		return_t<T...> operator() (const tuple & op)const {
			std::cout << "tuple" << std::endl;
			v_tuple<T...> ret;
			for (auto& elem : op.elems) {
				auto value = boost::apply_visitor(*this, elem);
				ret.tuple.push_back(value);
			}
			return ret;
		}

		template<class Expr>
		static return_t<T...> get(const Instance<T...>& i, const Expr& arg) {
			auto v = visitor_debug<Visitor_v, Visitor_f, T...>{ i };
			return boost::apply_visitor(v, arg);
		}

	};



}



#endif