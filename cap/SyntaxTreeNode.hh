#ifndef SYNTAX_TREE_NODE_HEADER
#define SYNTAX_TREE_NODE_HEADER

#include "Token.hh"

#include <memory>

namespace Cap
{

struct SyntaxTreeNode
{
	enum class Type
	{
		Assign,
		Access,

		Or,
		And,

		BitwiseOR,
		BitwiseAND,
		BitwiseXOR,
		BitwiseShiftLeft,
		BitwiseShiftRight,

		Equal,
		Inequal,

		Less,
		Greater,
		LessEqual,
		GreaterEqual,

		Addition,
		Subtraction,
		Multiplication,
		Division,
		Modulus,
		Power,

		Not,
		Reference,
		BitwiseNOT,
		UnaryPositive,
		UnaryNegative,

		None,

		Subscript,
		Call,

		Range,
		Ternary,
		Comma,

		Array,
		Parentheses,
		AnonFunction,
		Parameters,

		If,
		When,
		While,
		Return,
		Variable,
		Expression,
		Value,
	};

	SyntaxTreeNode(SyntaxTreeNode* parent) : parent(parent)
	{
	}

	SyntaxTreeNode(SyntaxTreeNode* parent, Token* value, Type t)
		: parent(parent), value(value), type(t)
	{
	}
	
	SyntaxTreeNode(SyntaxTreeNode&& rhs)
		:	parent(rhs.parent), value(rhs.value), type(rhs.type)
	{
		rhs.parent = nullptr;
		rhs.value = nullptr;
	}

	SyntaxTreeNode* parent = nullptr;
	Token* value = nullptr;
	Type type;

	//	FIXME Instead of allocating each node separately, pool them in the current scope
	std::shared_ptr <SyntaxTreeNode> right;
	std::shared_ptr <SyntaxTreeNode> left;

	static const char* getTypeString(Type t);
	const char* getTypeString() { return getTypeString(type); }
	void list(unsigned indent = 0);
};

}

#endif
