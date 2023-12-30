#include <cap/node/Expression.hh>

#include <cassert>

namespace cap
{

Expression::Expression(Type type, Token token)
	: Node(Node::Type::Expression, token), type(type)
{
}

bool Expression::replaceExpression(std::shared_ptr <Expression> node)
{
	assert(false && "called Expression::replaceExpression");
}

bool Expression::handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser)
{
	assert(false && "called Expression::handleExpressionNode");
}

//TypeDefinition& Expression::getResultType()
//{
//	printf("NOTE: Return invalid type from Expression::getResultType()\n");
//	return Type::getInvalid();
//}

void Expression::setBraceDepth(size_t depth)
{
	braceDepth = depth;
}

bool Expression::isOnSameLevel(std::shared_ptr <Expression> expr)
{
	return braceDepth == expr->braceDepth;
}

}