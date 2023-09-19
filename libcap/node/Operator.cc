#include <cap/node/Operator.hh>

namespace cap
{

bool Operator::handleToken(Token&& token, ParserState& state)
{
	printf("[Operator] Handle token '%s'\n", token.getString().c_str());

	auto result = parseToken(std::move(token), state);

	if(!result)
	{
		return false;
	}

	else if(result->isValue())
	{
		if(!handleValue(std::static_pointer_cast <Value> (result), state))
			return false;
	}

	else if(result->isOperator())
	{
		// Adopt the created operator.
		adopt(result);

		auto op = std::static_pointer_cast <Operator> (result);
		printf("Result is operator %s\n", op->getTypeString());

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
			printf("NEW OPERATOR HAS LOWER OR SAME PRECEDENCE\n");
			if(!handleLowerPrecedence(op, state))
				return false;
		}
			
		state.node = result;
	}

	else
	{
		printf("??? Weird result\n");
		return false;
	}

	return true;
}

}
