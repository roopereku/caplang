#include <cap/node/FunctionDefinition.hh>
#include <cap/node/FunctionSignature.hh>
#include <cap/node/ExplicitReturnType.hh>

#include <cap/event/ErrorMessage.hh>

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

bool FunctionDefinition::initializeSignature(Validator& validator)
{
	assert(!signature);
	signature = std::make_shared <FunctionSignature> (shared_from_this()->as <FunctionDefinition> ());
	adopt(signature);

	// TODO: This could be done in FunctionSignature.

	// Look for an explicit return type.
	auto current = getRoot();

	while(current)
	{
		// Check if the current node is an explicit return type.
		if(current->type == Node::Type::Expression &&
			current->as <Expression> ()->type == Expression::Type::Root &&
			current->as <ExpressionRoot> ()->type == ExpressionRoot::Type::ExplicitReturnType)
		{
			// Try to validate the explicit return type.
			if(!validator.validateNode(current))
			{
				return false;
			}

			// Get the result type and make sure that it's valid.
			auto result = current->as <ExplicitReturnType> ()->getResultType();
			if(result.expired())
			{
				validator.events.emit(ErrorMessage("Bug: Result type was empty", current->token));
				return false;
			}

			// Save the result type if it was valid.
			signature->returnType = result.lock();
		}

		current = current->getNext();
	}

	return true;
}

}
