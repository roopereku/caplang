#include <cap/node/FunctionSignature.hh>
#include <cap/node/PrimitiveType.hh>

#include <cap/event/ErrorMessage.hh>

#include <cap/Validator.hh>

namespace cap
{

FunctionSignature::FunctionSignature(std::shared_ptr <FunctionDefinition> target)
	: TypeDefinition(target->name, TypeDefinition::Type::FunctionSignature), target(target)
{
	// Default to to void.
	returnType = PrimitiveType::getVoid();

	// Count the amount of parameters this function has.
	auto current = target->getRoot();
	while(current &&
		current->type == Node::Type::Expression &&
		current->as <Expression> ()->type == Expression::Type::Root &&
		current->as <ExpressionRoot> ()->type == ExpressionRoot::Type::ParameterDefinition)
	{
		parameterCount++;
		current = current->getNext();
	}
}

std::shared_ptr <TypeDefinition> FunctionSignature::getReturnType()
{
	return returnType;
}

bool FunctionSignature::setReturnType(std::shared_ptr <TypeDefinition> node, Validator& validator)
{
	// If the return type doesn't have a default value (Has been set once), validate the new type.
	if(!returnTypeIsDefault && returnType)
	{
		// Make sure that the result type matches the current return type if any.
		if(node != returnType)
		{
			validator.events.emit(ErrorMessage("Mismatching return type", node->token));
			return false;
		}
	}

	returnType = node;
	returnTypeIsDefault = false;

	return true;
}

unsigned FunctionSignature::getParameterCount()
{
	return parameterCount;
}

std::shared_ptr <ParameterDefinition> FunctionSignature::getParameter(unsigned index)
{
	auto current = target.lock()->getRoot();
	unsigned currentIndex = 0;

	while(current &&
		current->type == Node::Type::Expression &&
		current->as <Expression> ()->type == Expression::Type::Root &&
		current->as <ExpressionRoot> ()->type == ExpressionRoot::Type::ParameterDefinition)
	{
		if(index == currentIndex)
		{
			return current->as <ParameterDefinition> ();
		}

		current = current->getNext();
		currentIndex++;
	}

	return nullptr;
}

}
