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
		// Adopt the created operator.
		adopt(result);

		auto op = std::static_pointer_cast <Operator> (result);
		printf("Result is operator %s\n", op->getTypeString());

		// The concept of high and low is inverted here as 0 is the
		// highest priority, therefore highest precedence.
		if(op->getPrecedence() < getPrecedence())
		{
			printf("NEW OPERATOR HAS HIGHER PRECEDENCE\n");

			if(op->isTwoSidedOperator())
			{
				printf("Set '%s' to lhs of new node '%s'\n", right->getToken().c_str(), op->getTypeString());
				printf("Set new node '%s' to rhs of '%s'\n", result->getToken().c_str(), getTypeString());

				auto twoSided = std::static_pointer_cast <TwoSidedOperator> (op);

				// Move the rhs of the current node to the lhs of the new node.
				twoSided->adopt(right);
				twoSided->left = std::move(right);
			}

			else if(op->isOneSidedOperator())
			{
				printf("TODO: Implement one sided operator higher precedence\n");
				return false;
			}

			else
			{
				printf("??? Weird operator type\n");
				return false;
			}

			// The rhs of current becomes the new node.
			right = std::move(op);
		}

		else
		{
			printf("NEW OPERATOR HAS LOWER OR SAME PRECEDENCE\n");

			// Move the rhs of the current node to the lhs of the new node.
			if(op->isTwoSidedOperator())
			{
				printf("Set current node '%s' to lhs of new node '%s'\n", getToken().c_str(), op->getTypeString());
				printf("Adopt new node '%s' as rhs of parent'%s'\n", op->getTypeString(), parent->getToken().c_str());

				auto twoSided = std::static_pointer_cast <TwoSidedOperator> (op);
				twoSided->left = std::static_pointer_cast <Expression> (shared_from_this());

				if(parent->isExpression())
				{
					auto parentExpr = std::static_pointer_cast <Expression> (parent);

					if(parentExpr->isOperator())
					{
						auto parentOp = std::static_pointer_cast <Operator> (parentExpr);

						if(parentOp->isTwoSidedOperator())
						{
							parent->adopt(twoSided);
							std::static_pointer_cast <TwoSidedOperator> (parentOp)->right = twoSided;
						}

						else
						{
							printf("TODO: Implement one sided parent\n");
							return false;
						}
					}

					else
					{
						printf("??? Parent isn't operator\n");
						return false;
					}
				}

				else
				{
					printf("??? Parent isn't expression\n");
					return false;
				}
			}

			else if(op->isOneSidedOperator())
			{
				printf("TODO: Implement one sided operator lower precedence\n");
				return false;
			}

			else
			{
				printf("??? Weird operator type\n");
				return false;
			}
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
		case Type::Division: return "Division";
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
		case Type::Division: return 5;
	}

	return -1;
}

bool TwoSidedOperator::isTwoSidedOperator()
{
	return true;
}

}
