#ifndef FUNCTION_HEADER
#define FUNCTION_HEADER

#include "Token.hh"
#include "Variable.hh"

#include <vector>

namespace Cap
{

struct Scope;
struct Function
{
	Token* name;
	Token* returnType;

	std::vector <Variable> parameters;
	Scope* scope;
};

}

#endif
