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
	validator.events.emit(DebugMessage("VALIDATING FUNCTION " + name.getString(), token));

	// Ensure the signature exists.
	if(!signature)
	{
		signature = std::make_shared <FunctionSignature> (shared_from_this()->as <FunctionDefinition> ());
		adopt(signature);
	}

	if(!validator.validateNode(getRoot()))
	{
		return false;
	}

	complete();
	return true;
}

}
