#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/CallOperator.hh>
#include <cap/node/FunctionSignature.hh>

#include <cap/event/ErrorMessage.hh>

#include <cap/Validator.hh>

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

bool OneSidedOperator::validate(Validator& validator)
{
	bool resultFromExpression = true;

	// If this is a call operator, resolve the call target and use the
	// function return type as the result type of this operator node.
	if(type == OneSidedOperator::Type::Call)
	{
		validator.events.emit(ErrorMessage("Resolve target of call operator", token));
		// If there is a target to resolve, resolve it.
		auto definition = validator.resolveDefinition(shared_from_this()->as <CallOperator> ()->getTarget());
		if(!definition)
		{
			return false;
		}

		// TODO: Support other callables.
		if(definition.getType() != Reference::Type::FunctionDefinition)
		{
			validator.events.emit(ErrorMessage("Unable to call non-function", token));
			return false;
		}

		auto signature = validator.getDefinitionType(definition);
		if(!signature)
		{
			return false;
		}

		// The result type of the call operator becomes the return value of the function.
		auto returnType = signature->as <FunctionSignature> ()->getReturnType();
		setResultType(returnType);

		resultFromExpression = false;
	}

	else if(type == OneSidedOperator::Type::Subscript)
	{
		assert(false && "Subscript not implemented");
	}

	// Only if there is an expression, validate it. In case such as
	// CallOperator there might be no expression due to empty brackets.
	if(expression)
	{
		// Validate the expression of the one sided operator.
		if(!validator.validateNode(expression))
		{
			return false;
		}
	}

	// Use the type of the expression node.
	if(resultFromExpression)
	{
		setResultType(expression->getResultType().lock());
	}

	// TODO: Make sure that the expression node supports the given operator.
	return true;
}

}
