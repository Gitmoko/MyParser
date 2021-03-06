#pragma once

#include"MyParser_Boost_Cfg.h"
#include"MyParserSyntax.h"

#define ParserOpeq ==
#define ParserOpnoteq !=
#define ParserOpand_ &&
#define ParserOpor_ ||
#define ParserOprelless <
#define ParserOprelmore >
#define ParserOpreleqless <=
#define ParserOpreleqmore >=
#define ParserOpadd +
#define ParserOpsub -
#define ParserOpmul *
#define ParserOpdiv /

#define ParserUOpplus +
#define ParserUOpminus -
#define ParserUOpnot_ !

#define ParserMakeOp(x) ParserOp##x

#define LiteralOpImpl(x) #x;
#define LiteralOp(x) LiteralOpImpl(x);


#define MakeCalculaters(x)\
    template<>\
	class Calculaters<MyParser::operators::x >{\
		public:\
		static std::string Symbol(){\
			return LiteralOp(ParserMakeOp(x));\
		}\
		static std::string Name(){\
			return #x;\
		}\
        template<class T1,class T2>\
		constexpr static auto Calc(T1 t1, T2 t2) {\
			return t1 ParserMakeOp(x) t2;\
		}\
	}

#define ParserMakeUOp(x) ParserUOp##x
#define MakeUCalculaters(x)\
template<>\
class UnaryCalculaters<MyParser::unary_operators::x> {\
public:\
	static std::string Symbol(){\
		return LiteralOp(ParserMakeUOp(x));\
	}\
	static std::string Name() {\
		return #x;\
	}\
	template<class T1>\
	constexpr static auto Calc(T1 t1) {\
		return ParserMakeUOp(x) t1;\
	}\
}\



namespace MyParser {

	template<MyParser::operators Op>
	class Calculaters;

	MakeCalculaters(add);
	MakeCalculaters(eq);
	MakeCalculaters(noteq);
	MakeCalculaters(and_);
	MakeCalculaters(or_);
	MakeCalculaters(relless);
	MakeCalculaters(relmore);
	MakeCalculaters(releqless);
	MakeCalculaters(releqmore);
	MakeCalculaters(sub);
	MakeCalculaters(mul);
	MakeCalculaters(div);

	template<MyParser::unary_operators Op>
	class UnaryCalculaters;

	MakeUCalculaters(plus);
	MakeUCalculaters(minus);
	MakeUCalculaters(not_);

}
