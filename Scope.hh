#ifndef SCOPE_HEADER
#define SCOPE_HEADER

#include "Token.hh"
#include "Variable.hh"
#include "Function.hh"

#include <vector>

namespace Cap
{

enum class ScopeContext
{
	Namespace,
	Function,
	Block,
	Type,
};

struct Scope
{
	Scope(Token* name, ScopeContext ctx) : name(name), ctx(ctx)
	{
	}

	Token* name;
	ScopeContext ctx;
	
	std::vector <Scope> types;
	std::vector <Variable> variables;
	std::vector <Function> functions;
};

}

#endif
