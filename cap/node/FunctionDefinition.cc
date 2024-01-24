#include <cap/node/FunctionDefinition.hh>
#include <cap/node/FunctionSignature.hh>
#include <cap/node/ExplicitReturnType.hh>

#include <cap/event/ErrorMessage.hh>
#include <cap/event/DebugMessage.hh>

#include <cassert>

namespace cap
{

FunctionDefinition::FunctionDefinition(Token name)
	: ScopeDefinition(Type::FunctionDefinition, name)
{
}

std::shared_ptr <FunctionSignature> FunctionDefinition::getSignature()
{
	return signature;
}

bool FunctionDefinition::validate(Validator& validator)
{
	// Ensure the signature exists.
	if(!signature)
	{
		signature = std::make_shared <FunctionSignature> (shared_from_this()->as <FunctionDefinition> ());
		adopt(signature);
	}

	// Validate the function contents.
	if(!validator.validateNode(getRoot()))
	{
		return false;
	}

	// If this function is a constructor, return the parent class type.
	if(isConstructor())
	{
		auto parentClass = getParent().lock();
		assert(parentClass->type == Node::Type::ScopeDefinition);
		assert(parentClass->as <ScopeDefinition> ()->type == ScopeDefinition::Type::TypeDefinition);

		signature->returnType = parentClass->as <TypeDefinition> ();
	}

	complete();
	return true;
}

}
