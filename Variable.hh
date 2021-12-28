#ifndef VARIABLE_HEADER
#define VARIABLE_HEADER

#include "Token.hh"

namespace Cap
{

struct Variable
{
	Token* name;
	Token* typeName;
	Token* initialValue;
};

}

#endif
