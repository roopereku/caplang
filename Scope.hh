#ifndef SCOPE_HEADER
#define SCOPE_HEADER

#include "SyntaxTreeNode.hh"
#include "Token.hh"
#include "Variable.hh"
#include "Function.hh"
#include "Type.hh"

#include <vector>
#include <memory>

namespace Cap
{

enum class ScopeContext
{
	Namespace,
	Function,
	Block,
	Type,
};

class Scope
{
public:
	Scope(Scope* parent, ScopeContext ctx, size_t begin, size_t end)
		:	parent(parent), ctx(ctx), begin(begin), end(end),
			root(nullptr), node(&root)
	{
		root.type = SyntaxTreeNode::Type::Expression;
	}

	Function& addFunction(Token* name, size_t begin, size_t end);
	Type& addType(Token* name, size_t begin, size_t end);
	Variable& addVariable(Token* name);

	Scope* parent;
	ScopeContext ctx;

	size_t begin;
	size_t end;
	
	SyntaxTreeNode root;
	SyntaxTreeNode* node;

private:
	std::vector <Type> types;
	std::vector <Variable> variables;
	std::vector <Function> functions;
};

}

#endif
