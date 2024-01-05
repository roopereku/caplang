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

void Expression::setResultType(std::shared_ptr <TypeDefinition> node)
{
	resultType = node;
}

std::weak_ptr <TypeDefinition> Expression::getResultType()
{
	return resultType;
}

}
