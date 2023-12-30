#include <cap/node/Operator.hh>

#include <cap/event/DebugMessage.hh>

#include <cassert>

namespace cap
{

bool Operator::handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser)
{
	assert(node);
	parser.events.emit(DebugMessage(std::string("Handle expression in operator ") + getTypeString(), node->token));

	// Adopt the expression node.
	adopt(node);

	if(node->type == Expression::Type::Value)
	{
		if(!handleValue(node))
		{
			return false;
		}
	}

	// Is the new node an operator.
	else if(node->type == Expression::Type::Operator)
	{
		auto op = node->as <Operator> ();

		// The concept of high and low is inverted here as 0 is the
		// highest priority, therefore highest precedence.
		if(op->getPrecedence() < getPrecedence())
		{
			//printf("NEW OPERATOR HAS HIGHER PRECEDENCE\n");
			if(!handleHigherPrecedence(op))
			{
				return false;
			}
		}

		else
		{
			// If the precedence is not the same, make the parent the current node and try again.
			if(op->getPrecedence() != getPrecedence())
			{
				// TODO: Don't go to parent if outside brackets.
				parser.setCurrentNode(getParent().lock());
				return getParent().lock()->as <Expression> ()->handleExpressionNode(node, parser);
			}

			if(!handleSamePrecedence(op))
			{
				return false;
			}
		}
			
		parser.setCurrentNode(node);
	}

	else
	{
		assert(false && "node in Operator::handleExpressionNode() has a weird type");
	}

	return true;
}


}
