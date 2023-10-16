#include <cap/node/Operator.hh>

namespace cap
{

bool Operator::handleToken(Token&& token, ParserState& state)
{
	printf("[Operator] Handle token '%s'\n", token.getString().c_str());

	auto result = parseToken(std::move(token), state);
	return handleExpressionNode(result, state);
}

bool Operator::handleExpressionNode(std::shared_ptr <Expression> node, ParserState& state)
{
	if(!node)
	{
		return false;
	}

	else if(node->isValue() || node->isExpressionRoot())
	{
		//printf("Handle value '%s'\n", node->getToken().c_str());
		if(!handleValue((node), state))
			return false;
	}

	else if(node->isOperator())
	{
		// Adopt the created operator.
		adopt(node);

		auto op = node->as <Operator> ();
		printf("[%s] Result is operator %s\n", getTypeString(), op->getTypeString());

		// The concept of high and low is inverted here as 0 is the
		// highest priority, therefore highest precedence.
		if(op->getPrecedence() < getPrecedence())
		{
			printf("NEW OPERATOR HAS HIGHER PRECEDENCE\n");
			if(!handleHigherPrecedence(op, state))
				return false;
		}

		else
		{
			// If the precedence is not the same, make the parent the current node and try again.
			if(op->getPrecedence() != getPrecedence())
			{
				printf("NEW OPERATOR HAS LOWER PRECEDENCE\n");

				state.node = parent;
				return parent->as <Expression> ()->handleExpressionNode(node, state);
			}

			printf("NEW OPERATOR HAS SAME PRECEDENCE\n");
			if(!handleSamePrecedence(op, state))
				return false;
		}
			
		state.node = node;
	}

	else
	{
		printf("??? Weird result\n");
		return false;
	}

	return true;
}


}
