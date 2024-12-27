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

	std::array <std::wstring_view, 4> ops
	{
		L"+", L"-", L"*", L"/"
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
		case Type::Add:
		case Type::Subtract:
			return 12;

		case Type::Multiply:
		case Type::Divide:
			return 13;
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

const char* BinaryOperator::getTypeString()
{
	switch(type)
	{
		case Type::Add: return "Add";
		case Type::Subtract: return "Subtract";
		case Type::Multiply: return "Multiply";
		case Type::Divide: return "Divide";
	}

	return "(binaryop) ???";
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
