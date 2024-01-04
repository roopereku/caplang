#include <cap/node/Expression.hh>

#include <cassert>

namespace cap
{

Expression::Expression(Type type, Token token)
	: Node(Node::Type::Expression, token), type(type)
{
}

bool Expression::handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser)
{
	assert(false && "called Expression::handleExpressionNode");
}

std::shared_ptr <Expression> Expression::stealMostRecentValue()
{
	assert(false && "called Expression::stealMostRecentValue");
}

//TypeDefinition& Expression::getResultType()
//{
//	printf("NOTE: Return invalid type from Expression::getResultType()\n");
//	return Type::getInvalid();
//}

}
