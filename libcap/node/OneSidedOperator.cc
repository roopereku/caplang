#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>

namespace cap
{

struct OperatorEntry
{
	std::string_view str;
	OneSidedOperator::Type type;
};

static OperatorEntry operatorLookup[]
{
	{ "!", OneSidedOperator::Type::Not },
	{ "-", OneSidedOperator::Type::Negate },
	{ "~", OneSidedOperator::Type::BitwiseNot },
};

std::shared_ptr <Operator> OneSidedOperator::parse(Token&& token, ParserState& state)
{
	for(auto& entry : operatorLookup)
	{
		if(token == entry.str)
		{
			return std::make_shared <OneSidedOperator> (std::move(token), entry.type);
		}
	}

	return nullptr;
}

bool OneSidedOperator::applyCached(std::shared_ptr <Expression>&& cached)
{
	//printf("[OneSidedOperator] Apply cached '%s'\n", cached->getToken().c_str());

	if(expression)
	{
		printf("Expression is already set\n");
		return false;
	}

	expression = std::move(cached);
	adopt(expression);

	return true;
}

const char* OneSidedOperator::getTypeString()
{
	switch(type)
	{
		case Type::FunctionCall: return "Function call";
		case Type::Subscript: return "Subscript";

		case Type::Not: return "Not";
		case Type::Negate: return "Negate";
		case Type::BitwiseNot: return "Bitwise not";
	}

	return "???";
}

unsigned OneSidedOperator::getPrecedence()
{
	// Values from https://en.cppreference.com/w/cpp/language/operator_precedence
	switch(type)
	{
		case Type::FunctionCall: return 2;
		case Type::Subscript: return 2;

		case Type::Not: return 3;
		case Type::Negate: return 3;
		case Type::BitwiseNot: return 3;
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
		case Type::Subscript:
			return true;

		default:
			return false;
	}
}

bool OneSidedOperator::affectsNextValue()
{
	return !affectsPreviousValue();
}

bool OneSidedOperator::replaceExpression(std::shared_ptr <Expression> node)
{
	expression = node;
	adopt(expression);

	return true;
}

bool OneSidedOperator::handleSamePrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	auto parentExpr = parent->as <Expression> ();

	if(op->isTwoSided())
	{
		// Make this operator the lhs of the new operator.
		auto twoSided = op->as <TwoSidedOperator> ();
		twoSided->left = shared_from_this()->as <Expression> ();

		// Replace this operator with the new two sided operator.
		parentExpr->adopt(twoSided);
		if(!parentExpr->replaceExpression(twoSided))
			return false;

		twoSided->adopt(twoSided->left);
	}

	else if(op->isOneSided())
	{
		// Make this the expression of the new operator.
		auto oneSided = op->as <OneSidedOperator> ();
		oneSided->expression = shared_from_this()->as <Expression> ();

		// Replace this operator with the new one sided operator.
		parentExpr->adopt(oneSided);
		if(!parentExpr->replaceExpression(oneSided))
			return false;

		oneSided->adopt(oneSided->expression);
	}

	return true;
}

bool OneSidedOperator::handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	if(op->isTwoSided())
	{
		auto twoSided = op->as <TwoSidedOperator> ();

		// Move the expression of the current node to the lhs of the new node.
		twoSided->adopt(expression);
		twoSided->left = std::move(expression);
	}

	else if(op->isOneSided())
	{
		auto oneSided = op->as <OneSidedOperator> ();

		// If the new one sided operator affects the previous value (For an example abc[]),
		// make the one sided operator steal the expression of this operator. The new one
		// sided operator will become the new expression of this operator.
		if(oneSided->affectsPreviousValue())
		{
			oneSided->expression = expression;
			oneSided->adopt(oneSided->expression);
		}
	}

	// The rhs of current becomes the new node.
	expression = std::move(op);
	adopt(expression);

	return true;

}

bool OneSidedOperator::handleValue(std::shared_ptr <Expression> value, ParserState& state)
{
	expression = std::move(value);
	adopt(expression);

	return true;
}

}
