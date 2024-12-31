#include <cap/BinaryOperator.hh>
#include <cap/Source.hh>

#include <string_view>
#include <cassert>
#include <array>

namespace cap
{

std::shared_ptr <BinaryOperator> BinaryOperator::create(cap::ParserContext& ctx, Token token)
{
	// TODO: It could be faster if source provided a way to match against a list of strings.

	// TODO: Implement compound operators for operators that don't already
	// have an override. This is not implemented for ==, !=, <= and >=.
	std::array <std::wstring_view, 21> ops
	{
		L"=", L".",
		L"+", L"-", L"*", L"/", L"%", L"**",
		L"||", L"&&", L"<", L">", L"<=", L">=", L"==", L"!=",
		L"<<", L">>", L"&", L"|", L"^",
	};

	for(size_t i = 0; i < ops.size(); i++)
	{
		if(ctx.source.match(token, ops[i]))
		{
			return std::make_shared <BinaryOperator> (static_cast <Type> (i));
		}
	}
	
	return nullptr;
}

void BinaryOperator::handleValue(std::shared_ptr <Expression> node)
{
	if(!left)
	{
		left = node;
	}

	else if(!right)
	{
		right = node;
	}

	else
	{
		assert(false && "Tried to set a value for binary operator after lhs and rhs were set");
	}
}

bool BinaryOperator::isComplete() const
{
	return left && right;
}

unsigned BinaryOperator::getPrecedence()
{
	switch(type)
	{
		case Type::Assign: return 2;
		case Type::Or: return 3;
		case Type::And: return 4;
		case Type::BitwiseOr: return 5;
		case Type::BitwiseXor: return 6;
		case Type::BitwiseAnd: return 7;

		case Type::Equal:
		case Type::Inequal:
			return 8;

		case Type::Less:
		case Type::Greater:
		case Type::LessEqual:
		case Type::GreaterEqual:
			return 9;

		case Type::BitwiseShiftLeft:
		case Type::BitwiseShiftRight:
			return 11;

		case Type::Add:
		case Type::Subtract:
			return 12;

		case Type::Multiply:
		case Type::Divide:
		case Type::Modulus:
			return 13;

		case Type::Exponent: return 14;
		case Type::Access: return 16;
	}

	assert(false);
	return -1;
}

std::shared_ptr <Expression> BinaryOperator::getLeft()
{
	return left;
}

std::shared_ptr <Expression> BinaryOperator::getRight()
{
	return right;
}

const char* BinaryOperator::getTypeString(Type type)
{
	switch(type)
	{
		case Type::Assign: return "Assign";
		case Type::Access: return "Access";
		case Type::Add: return "Add";
		case Type::Subtract: return "Subtract";
		case Type::Multiply: return "Multiply";
		case Type::Divide: return "Divide";
		case Type::Modulus: return "Modulus";
		case Type::Exponent: return "Exponent";
		case Type::Or: return "Or";
		case Type::And: return "And";
		case Type::Less: return "Less";
		case Type::Greater: return "Greater";
		case Type::LessEqual: return "LessEqual";
		case Type::GreaterEqual: return "GreaterEqual";
		case Type::Equal: return "Equal";
		case Type::Inequal: return "Inequal";
		case Type::BitwiseShiftLeft: return "BitwiseShiftLeft";
		case Type::BitwiseShiftRight: return "BitwiseShiftRight";
		case Type::BitwiseAnd: return "BitwiseAnd";
		case Type::BitwiseOr: return "BitwiseOr";
		case Type::BitwiseXor: return "BitwiseXor";
	}

	return "(binaryop) ???";
}

const char* BinaryOperator::getTypeString()
{
	return getTypeString(type);
}

std::shared_ptr <Expression> BinaryOperator::stealLatestValue()
{
	if(right)
	{
		return std::move(right);
	}

	else if(left)
	{
		return std::move(left);
	}

	assert(false && "Cannot steal value from BinaryOperator since it has nothing");
	return nullptr;
}

}
