#ifndef MyParserH
#define MyParserH
#include"MyParserSemantics.h"

namespace MyParser {

	template<class Visitor_v, class Visitor_f, class...T>
	struct Visitor_v_base :boost::static_visitor<return_t<T...>> {
	public:

		using base_t = Visitor_v_base<Visitor_v, Visitor_f, T...>;
		using return_t_type = return_t<T...>;
		const std::string& name;
	public:

		Visitor_v_base(const std::string& name_) :name(name_) {}
	};

	template<class Visitor_v, class Visitor_f, class...T>
	struct Visitor_f_base :boost::static_visitor<return_t<T...>> {
	public:

		using base_t = Visitor_f_base<Visitor_v, Visitor_f, T...>;
		using return_t_type = return_t<T...>;
		using args_type = v_tuple<T...>;
		const std::string& name;
		const args_type& args;
	public:

		Visitor_f_base(const std::string& name_, const args_type& args_) :name(name_), args(args_) {}
	};

	template<class Visitor_v, class Visitor_f, class ...T>
	struct Parser_api {
	public:

		using v_tuple_type = v_tuple<T...>;
		using args_type = v_tuple_type;
		using return_t_type = return_t<T...>;
		using Instance_type = Instance<T...>;
		using My_t = Parser_api<Visitor_v, Visitor_f, T...>;
		using Visitor_v_base = Visitor_v_base<Visitor_v, Visitor_f, T...>;
		using Visitor_f_base = Visitor_f_base<Visitor_v, Visitor_f, T...>;
	public:

		static return_t_type Evaluate(const expression& expr, boost::variant<T...> i) {
			return_t_type ret;
			try {
				ret = visitor<Visitor_v, Visitor_f, T...>::get(Instance<T...>{i}, expr);
			}
			catch (bad_operand ex) {
				ex;
			}
			return ret;
		}

		static return_t<T...> Parse(const std::string& s, boost::variant<T...> i) {
			auto tree = parse_impl(s);
			if (!tree.empty()) {
				return_t<T...> ret;
				try {
					ret = visitor<Visitor_v, Visitor_f, T...>::get(Instance<T...>{i}, tree);
				}
				catch (bad_operand op) {
					;
				};
				return ret;
			}
			return{};
		}
		static return_t<T...> Parse_debug(const std::string& s, boost::variant<T...> i) {
			std::cout << "start:"<< s << std::endl;
			auto tree = parse_impl_debug(s);
			std::cout << "which:"<<tree.which() << std::endl;
			if (!tree.empty()) {
				auto ret = visitor_debug<Visitor_v, Visitor_f, T...>::get(Instance<T...>{i}, tree);
				return ret;
			}
			return{};

		}
	};

	namespace {
		expression Compile(const std::string& s) {
			expression tree;
			try {
				tree = parse_impl_debug(s);
			}
			catch (compile_failed err) {
				throw err;
			}
			return tree;
		}
	}

}
#endif