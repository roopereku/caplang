#include <cap/BracketOperator.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>

#include <cassert>
#include <array>

namespace cap
{

BracketOperator::BracketOperator(Type type)
	: Expression(Expression::Type::BracketOperator), innerRoot(std::make_shared <Root> ()), type(type)
{
}

std::shared_ptr <BracketOperator> BracketOperator::create(cap::ParserContext& ctx, Token token)
{
	// TODO: It could be faster if source provided a way to match against a list of strings.

	std::array <std::wstring_view, 3> ops
	{
		L"(", L"[", L"<"
	};

	for(size_t i = 0; i < ops.size(); i++)
	{
		if(ctx.source.match(token, ops[i]))
		{
			auto op = std::make_shared <BracketOperator> (static_cast <Type> (i));
			op->adopt(op->innerRoot);
			return op;
		}
	}
	
	return nullptr;
}

void BracketOperator::handleValue(std::shared_ptr <Expression> node)
{
	if(!context)
	{
		context = node;
	}

	else
	{
		innerRoot->handleValue(node);
	}
}

bool BracketOperator::isComplete() const
{
	// Bracket operators are always considered complete.
	// When the context doesn't exist, this node is considered
	// complete so a value will be immediately stolen for it.
	//
	// After the bracket operator has been created and assigned a
	// context we switch to the inner root. In order to not append
	// any further expression nodes for the inner root we still
	// indicate that this node is complete.
	return true;
}

unsigned BracketOperator::getPrecedence()
{
	return 17;
}

std::shared_ptr <Expression> BracketOperator::getContext() const
{
	return context;
}

std::shared_ptr <Expression::Root> BracketOperator::getInnerRoot() const
{
	return innerRoot;
}

BracketOperator::Type BracketOperator::getType() const
{
	return type;
}

const char* BracketOperator::getTypeString(Type type)
{
	switch(type)
	{
		case Type::Call: return "Call";
		case Type::Subscript: return "Subscript";
		case Type::Generic: return "Generic";
	}

	return "(bracketop) ???";
}

const char* BracketOperator::getTypeString()
{
	return getTypeString(type);
}

}
