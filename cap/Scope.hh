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
	InvalidAssign,
	InvalidOperand,
	TypingOutsideInit
};

class Scope
{
public:
	Scope(Scope* parent, ScopeContext ctx, size_t begin, size_t end);

	Function& addFunction(Token* name);
	Type& addType(Token* name, size_t begin, size_t end);
	Variable& addVariable(Token* name);

	Function* findFunction(Token* name);
	Variable* findVariable(Token* name);
	Type* findType(Token* name);

	size_t getFunctionCount() { return functions.size(); }
	SyntaxTreeNode* validate(ValidationResult& result);

	Scope* parent;
	ScopeContext ctx;

	size_t begin;
	size_t end;

	SyntaxTreeNode root;
	SyntaxTreeNode* node;

private:
	SyntaxTreeNode* validateNode(SyntaxTreeNode* n, ValidationResult& result);
	SyntaxTreeNode* findLeftmostNode(SyntaxTreeNode* n);

	std::vector <Type> types;
	std::vector <Scope> blocks;
	std::vector <Variable> variables;
	std::vector <Function> functions;
};

}

#endif
