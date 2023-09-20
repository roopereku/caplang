#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>

namespace cap
{

bool OneSidedOperator::applyCached(std::shared_ptr <Expression>&& cached)
{
	printf("[OneSidedOperator] Apply cached '%s'\n", cached->getToken().c_str());

	if(expression)
	{
		printf("Expression is already set\n");
		return false;
	}

	expression = std::move(cached);
	return true;
}

const char* OneSidedOperator::getTypeString()
{
	switch(type)
	{
		case Type::Negate: return "Negate";
		case Type::FunctionCall: return "Function call";
	}

	return "???";
}

unsigned OneSidedOperator::getPrecedence()
{
	// Values from https://en.cppreference.com/w/cpp/language/operator_precedence
	switch(type)
	{
		case Type::Negate: return 3;
		case Type::FunctionCall: return 2;
	}

	return -1;
}

bool OneSidedOperator::isOneSided()
{
	return true;
}

bool OneSidedOperator::affectsPreviousValue()
{
	switch(type)
	{
		case Type::FunctionCall:
			return true;

		default:
			return false;
	}
}

bool OneSidedOperator::affectsNextValue()
{
	return !affectsPreviousValue();
}

bool OneSidedOperator::handleLowerPrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	if(op->isTwoSided())
	{
		auto twoSided = std::static_pointer_cast <TwoSidedOperator> (op);

		printf("Steal lhs of '%s'\n", getTypeString());

		auto parentExpr = std::static_pointer_cast <Expression> (parent);
		twoSided->left = std::static_pointer_cast <Expression> (shared_from_this());

		parentExpr->adopt(twoSided);
		if(!parentExpr->replaceExpression(twoSided))
			return false;

		twoSided->adopt(twoSided->left);
	}

	return true;
}

bool OneSidedOperator::handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	printf("[OneSidedOperator] handleHigherPrecedence unimplemented\n");
	return false;
}

bool OneSidedOperator::handleValue(std::shared_ptr <Expression> value, ParserState& state)
{
	expression = std::move(value);

	// The current node should never contain a one sided operator with a value.
	state.node = state.node->getParent();

	return true;
}

}
