#ifndef SCOPE_HEADER
#define SCOPE_HEADER

#include "SyntaxTreeNode.hh"
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
		:	name(name), parent(parent), ctx(ctx), begin(begin), end(end),
			root(nullptr), node(&root)
	{
		root.type = SyntaxTreeNode::Type::Line;
	}

	Token* name;
	Scope* parent;
	ScopeContext ctx;

	size_t begin;
	size_t end;
	
	std::vector <Scope> types;
	std::vector <Variable> variables;
	std::vector <Function> functions;

	SyntaxTreeNode root;
	SyntaxTreeNode* node;
};

}

#endif
