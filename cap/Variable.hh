#ifndef VARIABLE_HEADER
#define VARIABLE_HEADER

#include "Token.hh"

namespace Cap
{

struct Type;
struct Variable
{
	Variable(Token* name) : name(name) {}

	Token* name;
	Type* type = nullptr;

	bool isFunctionPointer = false;
	bool isArray = false;
};

}

#endif
