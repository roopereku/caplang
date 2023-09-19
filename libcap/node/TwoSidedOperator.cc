#include <cap/node/TwoSidedOperator.hh>

namespace cap
{

bool TwoSidedOperator::handleLowerPrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	// Move the rhs of the current node to the lhs of the new node.
	if(op->isTwoSided())
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

				// If the parent is a two sided operator, make the new node its rhs value.
				if(parentOp->isTwoSided())
				{
					parent->adopt(twoSided);
					std::static_pointer_cast <TwoSidedOperator> (parentOp)->right = twoSided;
				}

				else
				{
					printf("[TwoSidedOperator::handleLowerPrecedence] One sided parent unimplemented\n");
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

	else if(op->isOneSided())
	{
		printf("[TwoSidedOperator::handleLowerPrecedence] One sided operators unimplemented\n");
		return false;
	}

	else
	{
		printf("??? Weird operator type\n");
		return false;
	}

	return true;
}

bool TwoSidedOperator::handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	if(op->isTwoSided())
	{
		auto twoSided = std::static_pointer_cast <TwoSidedOperator> (op);

		printf("Set '%s' to lhs of new node '%s'\n", right->getToken().c_str(), op->getTypeString());
		printf("Set new node '%s' to rhs of '%s'\n", twoSided->getToken().c_str(), getTypeString());

		// Move the rhs of the current node to the lhs of the new node.
		twoSided->adopt(right);
		twoSided->left = std::move(right);
	}

	else
	{
		printf("[TwoSidedOperator::handleHigherPrecedence] One sided operators unimplemented\n");
		return false;
	}

	// The rhs of current becomes the new node.
	right = std::move(op);

	return true;
}

bool TwoSidedOperator::handleValue(std::shared_ptr <Value> value, ParserState& state)
{
	if(!left)
	{
		printf("??? Left is missing\n");
		return false;
	}

	printf("Result is value. Save '%s' rhs of %s\n", value->getToken().c_str(), getTypeString());
	right = std::move(value);

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

bool TwoSidedOperator::isTwoSided()
{
	return true;
}

}
