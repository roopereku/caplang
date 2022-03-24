#ifndef FUNCTION_HEADER
#define FUNCTION_HEADER

#include "Token.hh"
#include "Variable.hh"
#include "SyntaxTreeNode.hh"

#include <vector>
#include <memory>

namespace Cap
{

struct Scope;
struct Function
{
	Function(Token* name) : name(name) {}

	Token* name;
	Token* returnType;

	std::shared_ptr <Scope> scope;
};

}

#endif
