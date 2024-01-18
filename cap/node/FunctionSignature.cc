#include <cap/node/FunctionSignature.hh>
#include <cap/node/PrimitiveType.hh>

#include <cap/event/ErrorMessage.hh>

#include <cap/Validator.hh>

namespace cap
{

FunctionSignature::FunctionSignature(std::shared_ptr <FunctionDefinition> target)
	: TypeDefinition(target->name, TypeDefinition::Type::FunctionSignature)
{
	// Default to to void.
	returnType = PrimitiveType::getVoid();
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

}
