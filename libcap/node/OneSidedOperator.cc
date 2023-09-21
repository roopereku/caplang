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

bool OneSidedOperator::handleLowerPrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	auto parentExpr = std::static_pointer_cast <Expression> (parent);

	if(op->isTwoSided())
	{
		auto twoSided = std::static_pointer_cast <TwoSidedOperator> (op);
		twoSided->left = std::static_pointer_cast <Expression> (shared_from_this());

		parentExpr->adopt(twoSided);
		if(!parentExpr->replaceExpression(twoSided))
			return false;

		twoSided->adopt(twoSided->left);
	}

	else if(op->isOneSided())
	{
		auto oneSided = std::static_pointer_cast <OneSidedOperator> (op);
		oneSided->expression = std::static_pointer_cast <Expression> (shared_from_this());

		parentExpr->adopt(oneSided);
		if(!parentExpr->replaceExpression(oneSided))
			return false;

		oneSided->adopt(oneSided->expression);
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
