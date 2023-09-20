#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/ExpressionRoot.hh>

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

			parentExpr->adopt(twoSided);
			if(!parentExpr->replaceExpression(twoSided))
				return false;
		}

		else
		{
			printf("??? Parent isn't expression\n");
			return false;
		}
	}

	else if(op->isOneSided())
	{
		auto oneSided = std::static_pointer_cast <OneSidedOperator> (op);

		//printf("[TwoSidedOperator::handleLowerPrecedence] One sided operators unimplemented\n");
		printf("Steal rhs of '%s'\n", getTypeString());

		auto parentExpr = std::static_pointer_cast <Expression> (parent);
		oneSided->expression = std::static_pointer_cast <Expression> (shared_from_this());

		parentExpr->adopt(oneSided);
		if(!parentExpr->replaceExpression(oneSided))
			return false;

		oneSided->adopt(oneSided->expression);

		return true;
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

	else if(op->isOneSided())
	{
		auto oneSided = std::static_pointer_cast <OneSidedOperator> (op);

		if(oneSided->affectsPreviousValue())
		{
			printf("Set '%s' to new node '%s'\n", right->getToken().c_str(), oneSided->getTypeString());
			printf("Set new node '%s' to rhs of '%s'\n", oneSided->getTypeString(), getTypeString());
			oneSided->handleExpressionNode(right, state);
			right = oneSided;
		}
	}

	// The rhs of current becomes the new node.
	right = std::move(op);

	return true;
}

bool TwoSidedOperator::handleValue(std::shared_ptr <Expression> value, ParserState& state)
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
		case Type::Access: return "Access";
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
		case Type::Access: return 2;
	}

	return -1;
}

bool TwoSidedOperator::isTwoSided()
{
	return true;
}

bool TwoSidedOperator::replaceExpression(std::shared_ptr <Expression> node)
{
	right = node;
	return true;
}

}
