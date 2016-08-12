#ifndef MyParserH
#define MyParserH
#include"MyParserSemantics.h"

namespace MyParser {

	template<class Visitor_v, class Visitor_f, class...T>
	struct Visitor_v_base :boost::static_visitor<return_t<T...>> {
	public:
		using base_t = Visitor_v_base<Visitor_v, Visitor_f, T...>;
		using return_t_type = return_t<T...>;
	};

	template<class Visitor_v, class Visitor_f, class...T>
	struct Visitor_f_base :boost::static_visitor<return_t<T...>> {
	public:

		using base_t = Visitor_f_base<Visitor_v, Visitor_f, T...>;
		using return_t_type = return_t<T...>;
		using args_type = v_tuple<T...>;
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
				throw ex;
			}
			return ret;
		}
		static return_t_type Evaluate_debug(const expression& expr, boost::variant<T...> i) {
			return_t_type ret;
			try {
				ret = visitor_debug<Visitor_v, Visitor_f, T...>::get(Instance<T...>{i}, expr);
			}
			catch (bad_operand ex) {
				throw ex;
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

	struct printer : public boost::static_visitor<void> {
		void operator()(const double&arg) {
			drawspace();
			std::cout << arg;
		}
		void operator()(const std::string& arg) {
			drawspace();
			std::cout << arg;
		}
		template<class ...T>
		void operator()(const MyParser::v_tuple<T...>& arg) {

			drawspace();
			std::cout << "[" << std::endl;

			depth++;
			for (auto&elem : arg.tuple) {
				boost::apply_visitor(*this, elem);
				std::cout << std::endl;
			}
			depth--;

			drawspace();
			std::cout << "]";
		}
		template<class...T>
		void operator()(const MyParser::Instance<T...>& arg) {
			drawspace();
			std::cout << "instance";
		}
		template<class Variant>
		static void printtree(const Variant& v) {
			auto p = printer{};
			boost::apply_visitor(p, v);
			std::cout << "\n" << std::endl;
		}

		void drawspace() {
			auto n = depth;
			while (n--)
				std::cout << "  ";
		}


	public:
		unsigned depth = 1;

	};

}
#endif