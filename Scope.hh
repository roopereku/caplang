#ifndef SCOPE_HEADER
#define SCOPE_HEADER

#include "SyntaxTreeNode.hh"
#include "Token.hh"
#include "Variable.hh"
#include "Function.hh"
#include "Type.hh"

#include <vector>
#include <utility>

namespace Cap
{

enum class ScopeContext
{
	Namespace,
	Function,
	Block,
	Type,
};

enum class ValidationResult
{
	Success,
	IdentifierNotFound,
	InvalidOperand,
	TypingOutsideInit
};

class Scope
{
public:
	Scope(Scope* parent, ScopeContext ctx, size_t begin, size_t end)
		:	parent(parent), ctx(ctx), begin(begin), end(end),
			root(nullptr), node(&root)
	{
		static size_t dd = 0;
		dd++;

		d = dd;
		root.type = SyntaxTreeNode::Type::Expression;
	}

	Function& addFunction(Token* name, size_t begin, size_t end);
	Type& addType(Token* name, size_t begin, size_t end);
	Variable& addVariable(Token* name);

	Function* findFunction(Token* name);
	Variable* findVariable(Token* name);
	Type* findType(Token* name);

	SyntaxTreeNode* validate(ValidationResult& result);

	Scope* parent;
	ScopeContext ctx;

	size_t begin;
	size_t end;

	size_t d;
	
	SyntaxTreeNode root;
	SyntaxTreeNode* node;

private:
	SyntaxTreeNode* validateNode(SyntaxTreeNode* n, ValidationResult& result);

	std::vector <Type> types;
	std::vector <Variable> variables;
	std::vector <Function> functions;
};

}

#endif
