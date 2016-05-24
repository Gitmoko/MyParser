#include<iostream>
#include<map>
#include"MyParser.h"

struct printer : public boost::static_visitor<void> {
	void operator()(double&arg){
		drawspace();
		std::cout << arg;
	}
	void operator()(std::string& arg) {
		drawspace();
		std::cout << arg;
	}
	template<class ...T>
	void operator()(MyParser::v_tuple<T...>& arg){
		for (auto&elem : arg.tuple) {
			drawspace();
			std::cout << "[" << std::endl;
			depth++;
			boost::apply_visitor(*this, elem);
			depth--;
			drawspace();
			std::cout << "[" << std::endl;
		}
	}
	template<class...T>
	void operator()(MyParser::Instance<T...>& arg) {
		drawspace();
		std::cout << "instance" << std::endl;
	}
	template<class Variant>
	static void printtree(Variant&& v){
		auto p = printer{};
		boost::apply_visitor(p, v);
		std::cout << std::endl;
	}

	void drawspace() {
		auto n = depth;
		while(n--)
			std::cout << " ";
	}


public:
	unsigned depth = 1;

};

struct C;
struct D;
struct visitor_v;
struct visitor_f;
using parser = MyParser::Parser_api<visitor_v, visitor_f, C*, D*>;
struct C {
	using F_t = std::map<std::string, std::function<parser::return_t_type(const parser::v_tuple_type&)>>;
	using V_t = std::map < std::string, parser::return_t_type>;
	F_t f;
	V_t v;

	C() {
		v["v"] = 4;
		f["f"] = [](const parser::v_tuple_type& arg) { return "C"; };
	}



};

struct D  {
	using F_t = std::map<std::string, std::function<parser::return_t_type(const parser::v_tuple_type&)>>;
	using V_t = std::map < std::string, parser::return_t_type>;
	F_t f;
	V_t v;
	C c;

	D() {
		v["v"] = 8;
		f["f"] = [](const parser::return_t_type& arg)->parser::return_t_type{ return "D"; };
		auto p = &c;
		f["child"] = [=](const parser::v_tuple_type& arg)->parser::return_t_type{return parser::Instance_type{ p }; };
	}

};

struct visitor_v : public boost::static_visitor<parser::return_t_type>{
	using ret = parser::return_t_type;
	std::string& name;
	visitor_v(std::string& name_) :name(name_){};
	ret operator()(C* arg) {
		if (arg->v.count(name)>0)
			return arg->v[name];
		return 0;
	}
	ret operator()(D* arg) {
		if (arg->v.count(name) > 0)
			return arg->v[name];
		return 0;
	}
	ret operator()(...) {
		return 0;
	}

};

struct visitor_f : public boost::static_visitor<parser::return_t_type> {
	using ret = parser::return_t_type;
	std::string& name;
	parser::v_tuple_type& args;
	visitor_f(std::string& name_, parser::v_tuple_type& args_ ) :name(name_),args(args_){};
	ret operator()(C* arg) {
		if (arg->f.count(name) > 0)
			return arg->f[name](args);
		return 0;
	}
	ret operator()(D* arg) {
		if (arg->f.count(name) > 0)
			return arg->f[name](args);
		return 0;
	}

	ret operator()(...) {
		return 0;
	}
};

int main() {
	std::string s;
	while (std::cin >> s, s != "q") {
		D d;
		auto ret = parser::Parse(s, { &d });
		if (!ret.empty()) {
			printer::printtree(ret);
		}
	}
}