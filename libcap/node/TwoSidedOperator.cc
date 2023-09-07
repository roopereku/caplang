#include <cap/node/TwoSidedOperator.hh>

namespace cap
{

bool TwoSidedOperator::handleToken(Token&& token, ParserState& state)
{
	printf("[TwoSidedOperator] Handle token '%s'\n", token.getString().c_str());

	auto result = parseToken(std::move(token), state);

	if(!result)
	{
		return false;
	}

	else if(result->isValue())
	{
		if(!left)
		{
			printf("??? Left is missing\n");
			return false;
		}

		printf("Result is value. Save '%s' rhs of %s\n", result->getToken().c_str(), getTypeString());
		right = std::move(result);
	}

	else if(result->isOperator())
	{
		auto op = std::static_pointer_cast <Operator> (result);
		printf("Result is operator %s\n", op->getTypeString());

		// The concept of high and low is inverted here as 0 is the
		// highest priority, therefore highest precedence.
		if(op->getPrecedence() < getPrecedence())
		{
			printf("New operator has higher precedence\n");

			if(op->isTwoSidedOperator())
			{
				printf("Set '%s' to lhs of new node '%s'\n", right->getToken().c_str(), op->getTypeString());
				printf("Set new node '%s' to rhs of '%s'\n", result->getToken().c_str(), getTypeString());

				// Move the rhs of the current node to the lhs of the new node.
				std::static_pointer_cast <TwoSidedOperator> (op)->left = std::move(right);
				right = result;
			}
		}

		else
		{
			printf("New operator has lower or same precedence\n");


			// Move the rhs of the current node to the lhs of the new node.
			std::static_pointer_cast <TwoSidedOperator> (op)->left = std::static_pointer_cast <Expression> (state.node);
			// TODO: Adopt the new node with a function called parent->adoptLeft / parent->adoptRight
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

bool TwoSidedOperator::applyCached(std::shared_ptr <Expression>&& cached)
{
	printf("[TwoSidedOperator] Apply cached '%s'\n", cached->getToken().c_str());

	if(left)
	{
		printf("Left is already set\n");
		return false;
	}

	left = std::move(cached);
	return true;
}

const char* TwoSidedOperator::getTypeString()
{
	switch(type)
	{
		case Type::Assignment: return "Assignment";
		case Type::Addition: return "Addition";
		case Type::Multiplication: return "Multiplication";
	}

	return "???";
}

unsigned TwoSidedOperator::getPrecedence()
{
	// Values from https://en.cppreference.com/w/cpp/language/operator_precedence
	switch(type)
	{
		case Type::Assignment: return 16;
		case Type::Addition: return 6;
		case Type::Multiplication: return 5;
	}

	return -1;
}

bool TwoSidedOperator::isTwoSidedOperator()
{
	return true;
}

}
