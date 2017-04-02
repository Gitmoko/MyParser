#include<iostream>
#include<map>
#include"MyParserAPI.h"
#include"MyParser_decode.h"

MyParser::expression parse_impl(const std::string& s);
MyParser::expression parse_impl_d(const std::string& s);


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
		v["hp"] = 4;
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
		v["hp"] = 8;
		f["f"] = [](const parser::args_type& arg)->parser::return_t_type{ return "D"; };
		auto p = &c;
		f["child"] = [=](const parser::args_type& arg)->parser::return_t_type{return parser::Instance_type{ p }; };
	}

};

struct visitor_v : public parser::Visitor_v_base{
	using ret = base_t::return_t_type;
	ret operator()(const std::string& name,C* arg) {
		std::cout <<"C's "<< name << std::endl;
		if (arg->v.count(name)>0)
			return arg->v[name];
		return 0;
	}
	ret operator()(const std::string& name,D* arg) {

		std::cout <<"D's "<< name << std::endl;
		if (arg->v.count(name) > 0)
			return arg->v[name];
		throw MyParser::bad_operand{};
		return 0;
	}
	ret operator()(const std::string& scope, std::string name) {
		return 0;
	}

};

struct visitor_f : public parser::Visitor_f_base {
	using ret = base_t::return_t_type;
	ret operator()(const std::string& name, base_t::args_type args ,C* arg) {

		std::cout <<"C's "<< name << std::endl;
		if (arg->f.count(name) > 0)
			return arg->f[name](args);
		return 0;
	}
	ret operator()(const std::string& name, base_t::args_type args,D* arg) {

		std::cout <<"D's "<< name << std::endl;
		if (arg->f.count(name) > 0)
			return arg->f[name](args);
		return 0;
	}

	 ret operator()(const std::string& scope,std::string name, base_t::args_type args) {
		return 0;
	}
};


int main() {
	std::string s;
	while (std::cout << ">", std::getline(std::cin, s), s != "q") {
		D d;
		MyParser::expression ast;
		try {
			ast = parse_impl_d(s);
			std::cout << MyParser::visitor_decode::get(ast) << std::endl;
		}
		catch (MyParser::compile_failed err) {
			std::cout <<"Expecting: "<< err.what << std::endl;
			std::cout <<"parsing here: "<< err.pos << std::endl;
		}
		try {
			auto ret = parser::Evaluate(ast, { &d });
			if (!ret.empty()) {
				MyParser::printer::printtree(ret);
			}
		}
		catch(std::exception& e){
			std::cout << e.what() << std::endl;
		}


	}
}