#include<iostream>
#include<map>
#include"MyParser.h"

struct printer : public boost::static_visitor<void> {
	void operator()(const double&arg){
		drawspace();
		std::cout << arg;
	}
	void operator()(const std::string& arg) {
		drawspace();
		std::cout << arg;
	}
	template<class ...T>
	void operator()(const MyParser::v_tuple<T...>& arg){
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
	void operator()(const MyParser::Instance<T...>& arg) {
		drawspace();
		std::cout << "instance" << std::endl;
	}
	template<class Variant>
	static void printtree(const Variant& v){
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
	using F_t = std::map<std::string, std::function<parser::return_t_type(const parser::args_type&)>>;
	using V_t = std::map < std::string, parser::return_t_type>;
	F_t f;
	V_t v;

	C() {
		v["v"] = 4;
		f["f"] = [](const parser::args_type& arg) { return "C"; };
	}



};

struct D  {
	using F_t = std::map<std::string, std::function<parser::return_t_type(const parser::args_type&)>>;
	using V_t = std::map < std::string, parser::return_t_type>;
	F_t f;
	V_t v;
	C c;

	D() {
		v["v"] = 8;
		f["f"] = [](const parser::args_type& arg)->parser::return_t_type{ return "D"; };
		auto p = &c;
		f["child"] = [=](const parser::args_type& arg)->parser::return_t_type{return parser::Instance_type{ p }; };
	}

};

struct visitor_v : public parser::Visitor_v_base{
	using ret = base_t::return_t_type;
	visitor_v(const std::string& name_) :Visitor_v_base(name_){};
	ret operator()(C* arg) {
		std::cout << name << std::endl;
		if (arg->v.count(name)>0)
			return arg->v[name];
		return 0;
	}
	ret operator()(D* arg) {

		std::cout << name << std::endl;
		if (arg->v.count(name) > 0)
			return arg->v[name];
		return 0;
	}
	ret operator()(...) {
		return 0;
	}

};

struct visitor_f : public parser::Visitor_f_base {
	using ret = base_t::return_t_type;
	visitor_f(const std::string& name_, base_t::args_type& args_ ) :parser::Visitor_f_base(name_,args_){}
	ret operator()(C* arg) {

		std::cout << name << std::endl;
		if (arg->f.count(name) > 0)
			return arg->f[name](args);
		return 0;
	}
	ret operator()(D* arg) {

		std::cout << name << std::endl;
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
	while (std::cout << ">",std::cin >> s, s != "q") {
		D d;
		auto ret = parser::Parse(s, { &d });
		if (!ret.empty()) {
			printer::printtree(ret);
		}
	}
}