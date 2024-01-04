#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>

#include <cassert>

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

std::shared_ptr <Operator> OneSidedOperator::parseToken(Token& token)
{
	for(auto& entry : operatorLookup)
	{
		if(token == entry.str)
		{
			return std::make_shared <OneSidedOperator> (token, entry.type);
		}
	}

	return nullptr;
}

const char* OneSidedOperator::getTypeString()
{
	switch(type)
	{
		case Type::Call: return "Call";
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
		case Type::Call: return 2;
		case Type::Subscript: return 2;

		case Type::Not: return 3;
		case Type::Negate: return 3;
		case Type::BitwiseNot: return 3;
	}

	return -1;
}

bool OneSidedOperator::affectsPreviousValue()
{
	switch(type)
	{
		case Type::Call:
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

bool OneSidedOperator::handleValue(std::shared_ptr <Expression>&& node)
{
	assert(!expression);

	expression = std::move(node);
	adopt(expression);

	return true;
}

std::shared_ptr <Expression> OneSidedOperator::stealMostRecentValue()
{
	auto expr = std::move(expression);
	return expr;
}

bool OneSidedOperator::isComplete()
{
	return static_cast <bool> (expression);
}

}
