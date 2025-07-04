#include <cap/UnaryOperator.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>

#include <string_view>
#include <cassert>
#include <array>

namespace cap
{

std::shared_ptr <UnaryOperator> UnaryOperator::createPrefix(cap::ParserContext& ctx, Token token)
{
	// TODO: It could be faster if source provided a way to match against a list of strings.

	std::array <std::wstring_view, 6> ops
	{
		L"-", L"!", L"~", L"*", L"++", L"--"
	};

	for(size_t i = 0; i < ops.size(); i++)
	{
		if(ctx.source.match(token, ops[i]))
		{
			return std::make_shared <UnaryOperator> (static_cast <Type> (i));
		}
	}
	
	return nullptr;
}

std::shared_ptr <UnaryOperator> UnaryOperator::createPostfix(cap::ParserContext& ctx, Token token)
{
	// TODO: It could be faster if source provided a way to match against a list of strings.

	std::array <std::wstring_view, 2> ops
	{
		L"++", L"--"
	};

	for(size_t i = 0; i < ops.size(); i++)
	{
		if(ctx.source.match(token, ops[i]))
		{
			constexpr size_t offset = static_cast <size_t> (Type::PostIncrement);
			return std::make_shared <UnaryOperator> (static_cast <Type> (offset + i));
		}
	}
	
	return nullptr;
}

void UnaryOperator::handleValue(std::shared_ptr <Expression> node)
{
	if(!expression)
	{
		expression = node;
	}

	else
	{
		assert(false && "Tried to set a value for unary operator after expression was set");
	}
}

bool UnaryOperator::isComplete() const
{
	return static_cast <bool> (expression);
}

unsigned UnaryOperator::getPrecedence()
{
	switch(type)
	{
		case Type::Negate: return preUnaryPrecedence;
		case Type::LogicalNot: return preUnaryPrecedence;
		case Type::BitwiseNot: return preUnaryPrecedence;
		case Type::ParseTime: return preUnaryPrecedence;

		case Type::PreIncrement: return preUnaryPrecedence;
		case Type::PreDecrement: return preUnaryPrecedence;

		// TODO: With this precedence it looks like post increment happens before pre increment.
		case Type::PostIncrement: return postUnaryPrecedence;
		case Type::PostDecrement: return postUnaryPrecedence;
	}

	assert(false);
	return -1;
}

UnaryOperator::Type UnaryOperator::getType()
{
	return type;
}

std::shared_ptr <Expression> UnaryOperator::getExpression()
{
	return expression;
}

const char* UnaryOperator::getTypeString(Type type)
{
	switch(type)
	{
		case Type::Negate: return "Negate";
		case Type::LogicalNot: return "LogicalNot";
		case Type::BitwiseNot: return "BitwiseNot";
		case Type::ParseTime: return "ParseTime";

		case Type::PreIncrement: return "PreIncrement";
		case Type::PreDecrement: return "PreDecrement";
		case Type::PostIncrement: return "PostIncrement";
		case Type::PostDecrement: return "PostDecrement";
	}

	return "(unaryop) ???";
}

const char* UnaryOperator::getTypeString() const
{
	return getTypeString(type);
}

std::shared_ptr <Expression> UnaryOperator::stealLatestValue()
{
	if(expression)
	{
		return std::move(expression);
	}

	assert(false && "Cannot steal value from UnaryOperator since it has nothing");
	return nullptr;
}

}
