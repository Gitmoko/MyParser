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
#include<exception>
#include<random>

#include <boost/utility/string_ref.hpp>
#include<boost/functional/hash.hpp>


#define STRING_VIEW(str) boost::string_ref{ str, sizeof(str)-1 }

namespace std
{
	template<>
	struct hash<boost::string_ref> {
		size_t operator()(boost::string_ref const& sr) const {
			return boost::hash_range(sr.begin(), sr.end());
		}
	};
}
namespace static_map {
	//https://gist.github.com/ldionne/f7ff609f00dc7025a213
	template <typename Key, typename Value, std::size_t N>
	struct map {
		std::array<std::pair<Key, Value>, N> storage_;

	private:
		constexpr void insert_impl(std::pair<Key, Value> const& pair,
			std::array<bool, N>& initialized)
		{
			std::size_t hash = std::hash<Key>{}(pair.first);
			auto start = std::next(initialized.begin(), hash % N);
			auto it = std::find(start, initialized.end(), false);

			if (it != initialized.end()) {
				std::size_t index = std::distance(initialized.begin(), it);
				storage_[index] = pair;
				initialized[index] = true;
				return;
			}

			it = std::find(initialized.begin(), start, false);
			if (it != start) {
				std::size_t index = std::distance(initialized.begin(), it);
				storage_[index] = pair;
				initialized[index] = true;
				return;
			}

			assert(false && "should never be reached");
		}

	public:
		template <typename ...K, typename ...V>
		constexpr explicit map(std::pair<K, V> const& ...pairs) {
			std::array<bool, N> initialized{}; // all false at the beginning
			int expand[] = {1, (insert_impl(pairs, initialized), int{})... };
			(void)expand;
		}

		constexpr Value const& at(Key const& key) const {
			std::size_t hash = std::hash<Key>{}(key);
			auto start = std::next(storage_.begin(), hash % N);
			auto it = std::find_if(start, storage_.end(), [&](auto const& p) {
				return p.first == key;
			});

			if (it != storage_.end())
				return it->second;

			it = std::find_if(storage_.begin(), start, [&](auto const& p) {
				return p.first == key;
			});
			if (it != start)
				return it->second;
			else
				throw std::out_of_range{ "no such key in the map" };
		}
	};


	template <bool ...b>
	struct and_
		: std::is_same<and_<b...>, and_<(b, true)...>>
	{ };

	template <typename Key, typename Value, typename ...Pairs>
	constexpr auto make_map(Pairs const& ...pairs) {
		static_assert(and_<std::is_same<Key, typename Pairs::first_type>::value...>::value,
			"make_map requires all keys to have the same type");

		static_assert(and_<std::is_same<Value, typename Pairs::second_type>::value...>::value,
			"make_map requires all values to have the same type");

		return map<Key, Value, sizeof...(Pairs)>{pairs...};
	}



}


namespace MyParser {

	struct bad_operand : public std::exception {
		std::string msg;

		bad_operand() :msg("bad_operand") {};

		template<class T>
		bad_operand(T _msg) :msg(std::string{ "bad_operand: " } +_msg) {}
		virtual const char* what()const {
			return  msg.c_str();
		}
	};

	struct bad_argument: public std::exception  {
		std::string msg;


		template<class T, class U >
		bad_argument(T _funcname, U _msg) :msg("\""+std::string{ _funcname }+"\": "+std::string{_msg}) {}
		virtual const char* what()const {
			return  msg.c_str();
		}
	};

	struct missing_symbol : public std::exception {
		std::string msg;
		template<class T, class U >
		missing_symbol(T _env, U _msg) :msg("\"" + std::string{ _env }+"\": " + std::string{ _msg }) {}
		virtual const char* what()const {
			return  msg.c_str();
		}
	};


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

		struct standards {
		private:
			standards() : stdfunc(func_map())
				, stdconstant(constant_map()) {}
		public:
			static standards& get() {
				static standards s;
				return s;
			}
			static auto func_map() {
				using return_type = return_t<T...>;
				using tuple_type = v_tuple<T...>;
				auto ret = static_map::make_map<boost::string_ref, std::function<return_type(tuple_type)>>(
					std::make_pair(STRING_VIEW("sin"), std::function<return_type(tuple_type)>([](tuple_type arg) {return std::sin(boost::get<double>(arg.tuple[0])); }))
					, std::make_pair(STRING_VIEW("cos"), std::function<return_type(tuple_type)>([](tuple_type arg) {return std::cos(boost::get<double>(arg.tuple[0])); }))
					, std::make_pair(STRING_VIEW("tan"), std::function<return_type(tuple_type)>([](tuple_type arg) {return std::tan(boost::get<double>(arg.tuple[0])); }))
					, std::make_pair(STRING_VIEW("pow"), std::function<return_type(tuple_type)>([](tuple_type arg) {return std::pow(boost::get<double>(arg.tuple[0]), boost::get<double>(arg.tuple[1])); }))
					, std::make_pair(STRING_VIEW("log"), std::function<return_type(tuple_type)>([](tuple_type arg) {return std::log(boost::get<double>(arg.tuple[1])) / std::log(boost::get<double>(arg.tuple[0])); }))
					, std::make_pair(STRING_VIEW("get"), std::function<return_type(tuple_type)>([](tuple_type arg) {return boost::get<tuple_type>(arg.tuple[1]).tuple[boost::get<double>(arg.tuple[0])]; }))
					, std::make_pair(STRING_VIEW("rot2D"), std::function<return_type(tuple_type)>([](tuple_type arg) {
					auto &vec = boost::get<tuple_type>(arg.tuple[0]).tuple;
					auto rad = boost::get<double>(arg.tuple[1]);
					auto radcos = std::cos(rad);
					auto radsin = std::sin(rad);
					auto x = boost::get<double>(vec[0]);
					auto y = boost::get<double>(vec[1]);
					return tuple_type{ tuple_type::mem_t{ x*radcos - y*radsin,x*radsin + y*radcos } }; }))
					, std::make_pair(STRING_VIEW("normalize"), std::function<return_type(tuple_type)>([](tuple_type arg) {
						auto sum = 0.0f;
						tuple_type ret{};
						try {
							ret = tuple_type{ boost::get<tuple_type>(arg.tuple[0]).tuple }.tuple;
							for (auto& elem : ret.tuple) {
								auto& data = boost::get<double>(elem);
								sum += data*data;
							}
							auto sum_sq = std::sqrt(sum);
							for (auto& elem : ret.tuple) {
								auto& data = boost::get<double>(elem);
								data /= sum_sq;
							}
						}
						catch (boost::bad_get e) {
							throw bad_argument{ "normalize", "arguments must be \"tuple of doubles\"" };
						}
						return ret;
					}))
						, std::make_pair(STRING_VIEW("IfElse"), std::function<return_type(tuple_type)>([](tuple_type arg) {
						auto cond = boost::get<double>(arg.tuple[0]);
						auto tn = arg.tuple[1];
						auto el = arg.tuple[2];
						return cond != 0 ? tn : el;
					}))
						, std::make_pair(STRING_VIEW("abs"), std::function<return_type(tuple_type)>([](tuple_type arg) {
						auto a = boost::get<double>(arg.tuple[0]);
						return std::abs(a);
					}))
						, std::make_pair(STRING_VIEW("floor"), std::function<return_type(tuple_type)>([](tuple_type arg) {
						auto a = boost::get<double>(arg.tuple[0]);
						return std::floor(a);
					}))
						, std::make_pair(STRING_VIEW("ceil"), std::function<return_type(tuple_type)>([](tuple_type arg) {
						auto a = boost::get<double>(arg.tuple[0]);
						return std::ceil(a);
					}))
						, std::make_pair(STRING_VIEW("length"), std::function<return_type(tuple_type)>([](tuple_type arg) {
						auto sum = 0.0f;
						auto sum_sq = 0.0;
						try {
							auto vec = tuple_type{ boost::get<tuple_type>(arg.tuple[0]).tuple }.tuple;
							for (auto& elem : vec) {
								auto& data = boost::get<double>(elem);
								sum += data*data;
							}
							sum_sq = std::sqrt(sum);
						}
						catch (boost::bad_get e) {
							throw bad_argument{ "length","arguments must be \"tuple of doubles\"" };
						}
						return sum_sq;
					}))
						, std::make_pair(STRING_VIEW("mod"), std::function<return_type(tuple_type)>([](tuple_type arg) {
						auto x = boost::get<double>(arg.tuple[0]);
						auto y = boost::get<double>(arg.tuple[1]);
						auto ret = std::fmod(x, y);
						return ret;
					}))
						, std::make_pair(STRING_VIEW("rand"), std::function<return_type(tuple_type)>([](tuple_type arg) {
						static std::mt19937 rand(std::random_device{}());
						auto min = boost::get<double>(arg.tuple[0]);
						auto max = boost::get<double>(arg.tuple[1]);
						std::uniform_real_distribution<double> dist{ min,max };
						auto ret = dist(rand);
						return ret;
					})));
				return ret;
			}
			static auto constant_map() {
				using return_type = return_t<T...>;
				auto ret = static_map::make_map<boost::string_ref, return_type>(
					std::make_pair(STRING_VIEW("PI"), return_type{ 3.1415926535897932384626 })
					, std::make_pair(STRING_VIEW("e"), return_type{ 2.718281828459045 })
					);
				return ret;
			}

		public:
			using stdfunc_t = decltype (standards::func_map());
			using stdconstant_t = decltype (standards::constant_map());
			stdfunc_t stdfunc;
			stdconstant_t  stdconstant;

		};

		using this_t = visitor<Visitor_v, Visitor_f, T...>;
		using Instance_type = Instance<T...>;
		using tuple_type = v_tuple<T...>;
		using return_type = return_t<T...>;

		const Instance<T...>& i;
		typename const this_t::standards& standards_;
		

		visitor(const Instance<T...>& i_) :i(i_),standards_(this_t::standards::get()) {}


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
				try {
					auto& stdfunc = standards_.stdfunc;
					return stdfunc.at(op.name)(tuple_type{ args });
				}
				catch (std::exception& e) {
					throw missing_symbol{ "std","\"" + op.name + "\" " + "isn't found" };
				}
			}
			else {
				auto ret = Visitor_f{}(scope.scope_name,op.name, tuple_type{ args });
				return ret;
			}
		}

		return_t<T...> operator()(const scope_operator<variable> &scope)const {
			auto op = scope.v;
			if (scope.scope_name.size() == 0) {
				try {
					auto& stdconstant = standards_.stdconstant;
					return stdconstant.at(op.name);
				}
				catch(std::exception& e){
					throw missing_symbol{ "std","\"" + op.name + "\" " + "isn't found" };
				}
			}
			auto ret = Visitor_v{}(scope.scope_name, op.name);
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


}



#endif