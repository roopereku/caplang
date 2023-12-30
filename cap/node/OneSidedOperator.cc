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

bool OneSidedOperator::handleHigherPrecedence(std::shared_ptr <Operator> op)
{
	if(op->type == Operator::Type::TwoSided)
	{
		auto twoSided = op->as <TwoSidedOperator> ();

		// Move the expression of the current node to the lhs of the new node.
		twoSided->adopt(expression);
		twoSided->left = std::move(expression);
	}

	else if(op->type == Operator::Type::OneSided)
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

bool OneSidedOperator::handleValue(std::shared_ptr <Expression>&& value)
{
	assert(!expression);

	expression = std::move(value);
	return true;
}

}
