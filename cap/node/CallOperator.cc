#include <cap/node/CallOperator.hh>
#include <cap/node/FunctionSignature.hh>

#include <cap/event/ErrorMessage.hh>

#include <cap/Validator.hh>

namespace cap
{

bool CallOperator::validate(Validator& validator)
{
	// Try to resolve the call target.
	target->setReference(validator.resolveDefinition(target));

	// Make sure that the target refers to something.
	auto definition = target->getReference();
	if(!definition)
	{
		validator.events.emit(ErrorMessage("BUG: Failed to find call target", token));
		return false;
	}

	// TODO: Support other callables.
	if(definition.getType() != Reference::Type::FunctionDefinition)
	{
		validator.events.emit(ErrorMessage("Unable to call non-function", token));
		return false;
	}

	setReference(definition);
	auto signature = validator.getDefinitionType(getReference());

	if(!signature)
	{
		validator.events.emit(ErrorMessage("BUG: No signature", token));
		return false;
	}

	// Ensure that the call parameters are valid.
	if(!OneSidedOperator::validate(validator))
	{
		return false;
	}

	// The result type of the call operator becomes the return value of the function.
	auto returnType = signature->as <FunctionSignature> ()->getReturnType();
	setResultType(returnType);

	return true;
}

void CallOperator::setTarget(std::shared_ptr <Expression>&& node)
{
	target = std::move(node);
	adopt(target);
}

std::shared_ptr <Expression> CallOperator::getTarget()
{
	return target;
}

}
