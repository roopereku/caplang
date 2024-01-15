#include <cap/node/FunctionSignature.hh>

namespace cap
{

FunctionSignature::FunctionSignature(std::shared_ptr <FunctionDefinition> target) :
	TypeDefinition(target->name, TypeDefinition::Type::FunctionSignature)
{
}

std::shared_ptr <TypeDefinition> FunctionSignature::getReturnType()
{
	return returnType;
}

void FunctionSignature::setReturnType(std::shared_ptr <TypeDefinition> node)
{
	returnType = node;
}

}
