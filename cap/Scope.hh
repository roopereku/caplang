#ifndef SCOPE_HEADER
#define SCOPE_HEADER

#include "SyntaxTreeNode.hh"
#include "Token.hh"
#include "Variable.hh"
#include "Function.hh"
#include "Type.hh"

#include <utility>
#include <string>
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
	bool validate();

	Scope* parent;
	ScopeContext ctx;

	size_t begin;
	size_t end;

	SyntaxTreeNode root;
	SyntaxTreeNode* node;

private:
	bool validateNode(SyntaxTreeNode* n);
	SyntaxTreeNode* findAppropriateNode(SyntaxTreeNode* n);

	struct NodeInfo
	{
		Type* t = nullptr;
		Variable* v = nullptr;
		Function* f = nullptr;

		SyntaxTreeNode* at = nullptr;
	};

	NodeInfo getNodeInfo(SyntaxTreeNode* n);
	NodeInfo getNodeInfoRecursive(SyntaxTreeNode* n);

	std::string getFullAccessName(SyntaxTreeNode* last);

	bool isNodeTypeName(NodeInfo& info);

	std::vector <Type> types;
	std::vector <Scope> blocks;
	std::vector <Variable> variables;
	std::vector <Function> functions;
};

}

#endif
