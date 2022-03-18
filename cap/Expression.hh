#ifndef EXPRESSION_HEADER
#define EXPRESSION_HEADER

#include "SyntaxTreeNode.hh"
#include "FlagList.hh"

namespace Cap
{

struct ExpressionPart
{
	SyntaxTreeNode::Type type;
	Token* value;
	bool used = false;
};

typedef FlagList <SyntaxTreeNode::Type> OperatorPrioty;
OperatorPrioty operatorsAtPriority(size_t priority);

}

#endif
