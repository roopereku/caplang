#include <cap/BracketOperator.hh>
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

	std::array <std::wstring_view, 2> ops
	{
		L"(", L"["
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
	// This node is considered complete if no context has been set to
	// make it steal a value immediately after the bracket operator has
	// been created. After the context exists, the inner root expression
	// determines the completeness.
	return !context || innerRoot->isComplete();
}

unsigned BracketOperator::getPrecedence()
{
	return 17;
}

std::shared_ptr <Expression> BracketOperator::getContext()
{
	return context;
}

std::shared_ptr <Expression::Root> BracketOperator::getInnerRoot()
{
	return innerRoot;
}

const char* BracketOperator::getTypeString()
{
	switch(type)
	{
		case Type::Call: return "Call";
		case Type::Subscript: return "Subscript";
	}

	return "(bracketop) ???";
}

}
