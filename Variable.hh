#ifndef VARIABLE_HEADER
#define VARIABLE_HEADER

#include "Token.hh"

namespace Cap
{

struct Variable
{
	Variable(Token* name) : name(name) {}

	Token* name;
	Token* typeName;
	Token* initialValue;
};

}

#endif
