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
	Scope(Token* name, Scope* parent, ScopeContext ctx, size_t begin, size_t end)
		: name(name), parent(parent), ctx(ctx), begin(begin), end(end)
	{
	}

	Token* name;
	Scope* parent;
	ScopeContext ctx;

	size_t begin;
	size_t end;
	
	std::vector <Scope> types;
	std::vector <Variable> variables;
	std::vector <Function> functions;
};

}

#endif
