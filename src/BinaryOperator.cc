#include <cap/BinaryOperator.hh>
#include <cap/ParserContext.hh>
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
	std::array <std::wstring_view, 22> ops
	{
		L"=", L".", L",",
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
		case Type::Comma: return commaPrecedence;
		case Type::Assign: return compoundPrecedence;
		case Type::Or: return binaryPrecedenceStart + 0;
		case Type::And: return binaryPrecedenceStart + 1;
		case Type::BitwiseOr: return binaryPrecedenceStart + 2;
		case Type::BitwiseXor: return binaryPrecedenceStart + 3;
		case Type::BitwiseAnd: return binaryPrecedenceStart + 4;

		case Type::Equal:
		case Type::Inequal:
			return binaryPrecedenceStart + 5;

		case Type::Less:
		case Type::Greater:
		case Type::LessEqual:
		case Type::GreaterEqual:
			return binaryPrecedenceStart + 6;

		case Type::BitwiseShiftLeft:
		case Type::BitwiseShiftRight:
			return binaryPrecedenceStart + 7;

		case Type::Add:
		case Type::Subtract:
			return binaryPrecedenceStart + 8;

		case Type::Multiply:
		case Type::Divide:
		case Type::Modulus:
			return binaryPrecedenceStart + 9;

		case Type::Exponent: return binaryPrecedenceStart + 10;
		case Type::Access: return binaryAccessPrecedence;
	}

	// Make sure that what's defined in Expression for the precedence boundaries is correct.
	static_assert(binaryPrecedenceStart + 10 == binaryPrecedenceEnd, "The last operator using binaryPrecedenceStart as a base doesn't match binaryPrecedenceEnd");

	assert(false);
	return -1;
}

BinaryOperator::Type BinaryOperator::getType()
{
	return type;
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
		case Type::Comma: return "Comma";
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

const char* BinaryOperator::getTypeString() const
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

BinaryOperator::ArgumentAccessor::ArgumentAccessor(std::shared_ptr <BinaryOperator> root)
	: op(root), current(op->getLeft())
{
}

std::shared_ptr <Expression> BinaryOperator::ArgumentAccessor::getNext()
{
	auto result = current;

	// Move to right and return left.
	if(current == op->getLeft())
	{
		current = op->getRight();
	}

	// Stop iteration after returning right.
	else if(current == op->getRight())
	{
		current = nullptr;
	}

	return result;
}

}
