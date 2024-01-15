#include <cap/node/FunctionSignature.hh>
#include <cap/node/PrimitiveType.hh>

namespace cap
{

FunctionSignature::FunctionSignature(std::shared_ptr <FunctionDefinition> target)
	: TypeDefinition(target->name, TypeDefinition::Type::FunctionSignature)
{
	// Default to a non-explicit void as it can be overriden.
	returnType = PrimitiveType::getVoid();
}

std::shared_ptr <TypeDefinition> FunctionSignature::getReturnType()
{
	return returnType;
}

void FunctionSignature::setReturnType(std::shared_ptr <TypeDefinition> node, bool isExplicit)
{
	returnType = node;
	returnTypeExplicit = isExplicit;
}

bool FunctionSignature::isReturnTypeExplicit()
{
	return returnTypeExplicit;
}

}
