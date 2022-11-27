#ifndef VARIABLE_HEADER
#define VARIABLE_HEADER

#include "Token.hh"

namespace Cap
{

struct Type;
struct Variable
{
	Variable(Token* name, unsigned depth)
		: name(name), depth(depth)
	{
	}

	Token* name;
	Type* type = nullptr;
	bool initialized = false;

	bool isFunctionPointer = false;
	bool isArray = false;

	unsigned depth;
};

}

#endif
