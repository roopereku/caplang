#include <cap/node/CallOperator.hh>
#include <cap/node/FunctionSignature.hh>
#include <cap/node/TwoSidedOperator.hh>

#include <cap/event/ErrorMessage.hh>

#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

bool CallOperator::validate(Validator& validator)
{
	// Ensure that the call parameters are valid.
	if(!OneSidedOperator::validate(validator))
	{
		return false;
	}

	// Get the expression root of the call parameters.
	std::shared_ptr <Expression> firstArgument =
		getExpression()->as <ExpressionRoot> ()->getRoot();

	// If there's an expression inside the call parenthesis, initialize
	// the parameter count to 1.
	unsigned passedArguments = static_cast <bool> (firstArgument);

	const auto isComma = [](std::shared_ptr <Expression> node)
	{
		return node->type == Expression::Type::Operator &&
				node->as <Operator> ()->type == Operator::Type::TwoSided &&
				node->as <TwoSidedOperator> ()->type == TwoSidedOperator::Type::Comma;
	};

	// If there's atleast one passed argument, check if there are more.
	if(passedArguments)
	{
		// Locate the first argument by traversing through possible commas.
		// Before this loop "firstArgument" should contain the first argument,
		// or a comma whose right side is the last argument.
		while(isComma(firstArgument))
		{
			// Switch to the left side of the comma. Eventually the first argument will be found.
			firstArgument = firstArgument->as <TwoSidedOperator> ()->getLeft();
			passedArguments++;
		}
	}

	auto definition = validator.resolveDefinition(target);
	while(definition)
	{
		// If the found definition is a function, check if the parameters match.
		if(definition.getType() == Reference::Type::FunctionDefinition)
		{
			auto function = definition.getReferred()->as <FunctionDefinition> ();
			
			// If the function isn't validated yet, try to validate it.
			if(!function->isValidationComplete() && !function->validate(validator))
			{
				return false;
			}

			// If this overload has the same number of parameters as are passed in,
			// check if this overload has the appropriate parameter types.
			auto signature = function->getSignature();
			if(passedArguments == signature->getParameterCount())
			{
				bool firstChecked = false;
				bool checkedRight = false;

				auto currentArgument = firstArgument;
				unsigned currentArgIndex = 0;

				while(currentArgIndex < passedArguments)
				{
					// If the first argument isn't checked yet, update the state
					// and keep currentArgument as firstArgument.
					if(!firstChecked)
					{
						firstChecked = true;
					}

					// If the first argument is checked, the right side value of the
					// parent comma contains the next argument.
					else if(!checkedRight)
					{
						auto comma = currentArgument->getParent().lock()->as <TwoSidedOperator> ();
						currentArgument = comma->getRight();
						checkedRight = true;
					}

					// If the right side value of the parent comma is checked, switch to the parent comma
					// This ensures that the next argument is checked in the next iteration.
					else
					{
						auto comma = currentArgument->getParent().lock();
						currentArgument = comma->as <Expression> ();

						// Don't do any type checking and check the right side value of the parent in the next iteration.
						checkedRight = false;
						continue;
					}

					// Get the current parameter.
					auto param = signature->getParameter(currentArgIndex);
					assert(param);

					auto paramType = param->getReference();
					assert(paramType.getType() == Reference::Type::TypeDefinition);

					// If the type of the given argument doesn't match the parameter type, stop matching.
					assert(!currentArgument->getResultType().expired());
					if(currentArgument->getResultType().lock() != paramType.getReferred()->as <TypeDefinition> ())
					{
						break;
					}

					currentArgIndex++;
				}

				// If all of the arguments match the parameters, this is the desired function overload.
				if(currentArgIndex == signature->getParameterCount())
				{
					target->setReference(definition);
					break;
				}
			}
		}

		// Find the next definition that matches the call target.
		auto name = definition.getReferredName().getStringView();
		definition = definition.getReferred()->findNextDefinition(name);
	}

	// If target still doesn't refer to a function, no valid overload could be found.
	definition = target->getReference();
	if(!definition)
	{
		validator.events.emit(ErrorMessage("No matching overload", token));
		return false;
	}

	setReference(definition);
	auto signature = validator.getDefinitionType(getReference());

	if(!signature)
	{
		validator.events.emit(ErrorMessage("BUG: No signature", token));
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
