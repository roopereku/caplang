#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>

#include <cassert>

namespace cap
{

struct OperatorEntry
{
	std::string_view str;
	TwoSidedOperator::Type type;
};

static OperatorEntry operatorLookup[]
{
	{ "=", TwoSidedOperator::Type::Assignment },
	{ "+", TwoSidedOperator::Type::Addition },
	{ "-", TwoSidedOperator::Type::Subtraction },
	{ "*", TwoSidedOperator::Type::Multiplication },
	{ "/", TwoSidedOperator::Type::Division },
	{ "%", TwoSidedOperator::Type::Modulus },
	{ ".", TwoSidedOperator::Type::Access },
	{ ",", TwoSidedOperator::Type::Comma },
	{ ">", TwoSidedOperator::Type::GreaterThan },
	{ ">=", TwoSidedOperator::Type::GreaterOrEqual },
	{ "<", TwoSidedOperator::Type::LessThan },
	{ "<=", TwoSidedOperator::Type::LessOrEqual },
	{ "==", TwoSidedOperator::Type::Equals },
	{ "!=", TwoSidedOperator::Type::NotEquals },
	{ "||", TwoSidedOperator::Type::Or },
	{ "&&", TwoSidedOperator::Type::And },
	{ "<<", TwoSidedOperator::Type::BitwiseShiftLeft },
	{ ">>", TwoSidedOperator::Type::BitwiseShiftRight },
	{ "&", TwoSidedOperator::Type::BitwiseAnd },
	{ "|", TwoSidedOperator::Type::BitwiseOr },
	{ "^", TwoSidedOperator::Type::BitwiseXor },
};

std::shared_ptr <Operator> TwoSidedOperator::parseToken(Token& token)
{
	for(auto& entry : operatorLookup)
	{
		if(token == entry.str)
		{
			return std::make_shared <TwoSidedOperator> (token, entry.type);
		}
	}

	return nullptr;
}

bool TwoSidedOperator::handleHigherPrecedence(std::shared_ptr <Operator> op)
{
	if(op->type == Operator::Type::TwoSided)
	{
		auto twoSided = op->as <TwoSidedOperator> ();

		// Move the rhs of the current node to the lhs of the new node.
		twoSided->adopt(right);
		twoSided->left = std::move(right);
	}

	else if(op->type == Operator::Type::OneSided)
	{
		auto oneSided = op->as <OneSidedOperator> ();

		// If the new one sided operator affects the previous value (For an example abc[]),
		// make the one sided operator steal the rhs value of this operator. The new one
		// sided operator will become the new rhs value of this operator.
		if(oneSided->affectsPreviousValue())
		{
			oneSided->expression = right;
			oneSided->adopt(right);
		}
	}

	// The rhs of current becomes the new node.
	right = std::move(op);
	adopt(right);

	return true;
}

bool TwoSidedOperator::handleValue(std::shared_ptr <Expression>&& value)
{
	if(!left)
	{
		left = std::move(value);
	}

	else if(!right)
	{
		right = std::move(value);
	}

	else
	{
		assert(false);
	}

	return true;
}

const char* TwoSidedOperator::getTypeString()
{
	switch(type)
	{
		case Type::Assignment: return "Assignment";
		case Type::Addition: return "Addition";
		case Type::Subtraction: return "Subtraction";
		case Type::Multiplication: return "Multiplication";
		case Type::Division: return "Division";
		case Type::Modulus: return "Modulus";
		case Type::Access: return "Access";
		case Type::Comma: return "Comma";

		case Type::GreaterThan: return "Greater than";
		case Type::GreaterOrEqual: return "Greater or equal";
		case Type::LessThan: return "Less than";
		case Type::LessOrEqual: return "Less or equal";
		case Type::Equals: return "Equals";
		case Type::NotEquals: return "Not equals";
		case Type::Or: return "Or";
		case Type::And: return "And";

		case Type::BitwiseShiftLeft: return "Bitwise shift left";
		case Type::BitwiseShiftRight: return "Bitwise shift right";
		case Type::BitwiseOr: return "Bitwise or";
		case Type::BitwiseAnd: return "Bitwise and";
		case Type::BitwiseXor: return "Bitwise xor";
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
		case Type::Subtraction: return 6;
		case Type::Multiplication: return 5;
		case Type::Division: return 5;
		case Type::Modulus: return 5;
		case Type::Access: return 2;
		case Type::Comma: return 17;

		case Type::GreaterThan: return 9;
		case Type::GreaterOrEqual: return 9;
		case Type::LessThan: return 9;
		case Type::LessOrEqual: return 9;
		case Type::Equals: return 10;
		case Type::NotEquals: return 10;
		case Type::Or: return 15;
		case Type::And: return 14;

		case Type::BitwiseShiftLeft: return 7;
		case Type::BitwiseShiftRight: return 7;
		case Type::BitwiseOr: return 13;
		case Type::BitwiseAnd: return 14;
		case Type::BitwiseXor: return 12;
	}

	return -1;
}

bool TwoSidedOperator::replaceExpression(std::shared_ptr <Expression> node)
{
	right = std::move(node);
	adopt(right);

	return true;
}

}
