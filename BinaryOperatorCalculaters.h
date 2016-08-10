#ifndef BinaryOperatorsCalculatersH
#define BinaryOperatorsCalculatersH
#include <string>
#include <algorithm>
#include<iostream>
#include "MyParserSyntax.h"

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

#define ParserMakeOp(x) ParserOp##x
#define MakeCalculaters(x)\
    template<>\
	class Calculaters<MyParser::operators::x >{\
		public:\
		static std::string Name(){\
			return #x;\
		}\
        template<class T1,class T2>\
		constexpr static auto Calc(T1 t1, T2 t2) {\
			return t1 ParserMakeOp(x) t2;\
		}\
	}

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

}

#endif