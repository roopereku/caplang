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

		Or,
		And,

		BitwiseOR,
		BitwiseAND,
		BitwiseNOT,
		BitwiseXOR,
		BitwiseShiftLeft,
		BitwiseShiftRight,

		Equal,
		Inequal,

		Not,
		Less,
		Greater,
		LessEqual,
		GreaterEqual,

		Addition,
		Subtraction,
		Multiplication,
		Division,
		Modulus,

		Access,
		Reference,
		UnaryPositive,
		UnaryNegative,

		Ternary,
		Condition,
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

	std::shared_ptr <SyntaxTreeNode> right;
	std::shared_ptr <SyntaxTreeNode> left;
};

}

#endif