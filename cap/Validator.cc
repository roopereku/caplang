#include <cap/Validator.hh>

#include <cap/node/Value.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/CallOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/VariableDefinition.hh>
#include <cap/node/FunctionDefinition.hh>
#include <cap/node/FunctionSignature.hh>
#include <cap/node/PrimitiveType.hh>

#include <cap/event/ErrorMessage.hh>
#include <cap/event/DebugMessage.hh>

#include <algorithm>
#include <cassert>

namespace cap
{

Validator::Validator(EventEmitter& events) : events(events)
{
}

bool Validator::validate(std::shared_ptr <Node> root)
{
	return validateNode(root);
}

bool Validator::validateNode(std::shared_ptr <Node> node)
{
	// TODO: Handle this in another way?
	if(!node)
	{
		return true;
	}

	switch(node->type)
	{
		case Node::Type::Expression:
		{
			if(!validateExpression(node->as <Expression> ()))
			{
				return false;
			}

			break;
		}

		case Node::Type::ScopeDefinition:
		{
			if(!validateScope(node->as <ScopeDefinition> ()))
			{
				return false;
			}

			break;
		}

		default:
		{
			events.emit(ErrorMessage(std::string("Validation unimplemented for ") + node->getTypeString(), node->token));
			return false;
		}
	}

	// Validate the next node.
	if(node->getNext() && !validateNode(node->getNext()))
	{
		return false;
	}

	return true;
}

bool Validator::validateExpression(std::shared_ptr <Expression> node)
{
	switch(node->type)
	{
		case Expression::Type::Root:
		{
			if(!validateExpressionRoot(node->as <ExpressionRoot> ()))
			{
				return false;
			}

			break;
		}

		case Expression::Type::Operator:
		{
			if(!validateOperator(node->as <Operator> ()))
			{
				return false;
			}

			break;
		}

		case Expression::Type::Value:
		{
			if(node->as <Value> ()->isGeneric())
			{
				events.emit(ErrorMessage("Unimplemented: Validation of generic values", node->token));
				return false;
			}

			if(node->token == Token::Type::Identifier)
			{
				events.emit(DebugMessage("Find definition of value " + node->token.getString(), node->token));

				auto definition = getDefinition(node->token, getCurrentScope(node));
				if(!definition)
				{
					return false;
				}

				node->setReference(definition);
			}

			// The non-identifier token should be conversible to a primitive type.
			else
			{
				auto primitiveType = PrimitiveType::fromToken(node->token);

				if(!primitiveType)
				{
					events.emit(ErrorMessage(std::string("Unable to get primitive type from ") + node->token.getTypeString(), node->token));
					return false;
				}

				node->setResultType(primitiveType);
			}

			break;
		}

		default:
		{
			events.emit(ErrorMessage(std::string("Validation unimplemented for expression ") + node->getTypeString(), node->token));
			return false;
		}
	}

	return true;
}

bool Validator::validateExpressionRoot(std::shared_ptr <ExpressionRoot> node)
{
	// If there's no expression, do nothing
	if(!node->getRoot())
	{
		return true;
	}

	// Validate the root expression node.
	if(!validateExpression(node->getRoot()))
	{
		return false;
	}

	// Set the result type of the expression root.
	node->setReference(node->getRoot()->getReference());
	node->setResultType(node->getRoot()->getResultType().lock());

	// Return statements might initialize a function return type.
	if(node->type == ExpressionRoot::Type::ReturnStatement)
	{
		// The current named scope has to be a function for return statements.
		auto scope = getCurrentNamedScope(node);
		if(scope->type != ScopeDefinition::Type::FunctionDefinition)
		{
			events.emit(ErrorMessage("Return statement outside a function", node->token));
			return false;
		}

		// If the expression of the return doesn't return any type, don't do checks.
		// Such could happen when an incomplete function is called.
		if(!node->getResultType().expired())
		{
			auto signature = scope->as <FunctionDefinition> ()->getSignature();
			auto resultType = node->getResultType().lock();

			// Disallow returning a void type.
			if(resultType == PrimitiveType::getVoid())
			{
				events.emit(ErrorMessage("Void used with return", node->token));
				return false;
			}

			// Only if the return type is explicitly set and isn't an implicit void, ensure the type matches.
			if(signature->isReturnTypeExplicit() || signature->getReturnType() != PrimitiveType::getVoid())
			{
				// Make sure that the result type matches the current return type if any.
				if(signature->getReturnType() && signature->getReturnType() != resultType)
				{
					events.emit(ErrorMessage("Mismatching return type", node->token));
					return false;
				}
			}

			// Set or update the return type.
			signature->setReturnType(resultType);
		}
	}

	// Explicit return types define the required the return type.
	else if(node->type == ExpressionRoot::Type::ExplicitReturnType)
	{
		// The current named scope has to be a function for explicit return types.
		auto scope = getCurrentNamedScope(node);
		if(scope->type != ScopeDefinition::Type::FunctionDefinition)
		{
			events.emit(ErrorMessage("BUG: Explicit return type in a non-function", node->token));
			return false;
		}

		// Only allow types as an explicit return type.
		auto referred = node->getReference();
		if(referred.getType() != Reference::Type::TypeDefinition)
		{
			events.emit(ErrorMessage("Non-type used as explicit return type", node->token));
			return false;
		}

		// Set the function return type to the type of the explicit return type.
		auto signature = scope->as <FunctionDefinition> ()->getSignature();
		signature->setReturnType(node->getResultType().lock(), true);
	}

	// Make sure that there aren't multiple initilizations of the same name in the same scope.
	else if(node->type == ExpressionRoot::Type::VariableDefinition ||
			node->type == ExpressionRoot::Type::ParameterDefinition)
	{
		// If there's a definition of the same name as node, other than node
		// that's in the same scope as node, there's a colliding initialization.
		auto definition = getDefinition(node->token, getCurrentScope(node));
		if(definition && definition.getReferred() != node &&
			getCurrentScope(node) == getCurrentScope(definition.getReferred()))
		{
			events.emit(ErrorMessage("Colliding initialization", node->token));
			return false;
		}
	}

	return true;
}

bool Validator::validateOperator(std::shared_ptr <Operator> node)
{
	if(!node->isComplete())
	{
		events.emit(ErrorMessage("Incomplete operator " + node->token.getString(), node->token));
		return false;
	}

	switch(node->type)
	{
		case Operator::Type::TwoSided:
		{
			auto twoSided = node->as <TwoSidedOperator> ();

			// The access operator has a special meaning.
			if(twoSided->type == TwoSidedOperator::Type::Access)
			{
				events.emit(ErrorMessage("Resolve access operator", node->token));
				auto resultDefinition = resolveAccess(twoSided);
				if(!resultDefinition)
				{
					return false;
				}

				// Use the type of the right node. This is because the rightmost node
				// contains the final access location.
				node->setResultType(twoSided->getRight()->getResultType().lock());

				return true;
			}

			// Try to validate both nodes of the two sided operator.
			if(!validateExpression(twoSided->getLeft()) ||
				!validateExpression(twoSided->getRight()))
			{
				return false;
			}

			// Use the type of the left node.
			node->setResultType(twoSided->getLeft()->getResultType().lock());

			// TODO: Make sure that the left node supports the given operator.
			// TODO: Make sure that the types of left and right are compatible.

			break;
		}

		case Operator::Type::OneSided:
		{
			auto oneSided = node->as <OneSidedOperator> ();
			bool resultFromExpression = true;

			// If this is a call operator, resolve the call target and use the
			// function return type as the result type of this operator node.
			if(oneSided->type == OneSidedOperator::Type::Call)
			{
				events.emit(ErrorMessage("Resolve target of call operator", node->token));
				// If there is a target to resolve, resolve it.
				auto definition = resolveDefinition(oneSided->as <CallOperator> ()->getTarget());
				if(!definition)
				{
					return false;
				}

				// TODO: Support other callables.
				if(definition.getType() != Reference::Type::FunctionDefinition)
				{
					events.emit(ErrorMessage("Unable to call non-function", node->token));
					return false;
				}

				auto signature = getDefinitionType(definition);
				if(!signature)
				{
					return false;
				}

				// The result type of the call operator becomes the return value of the function.
				auto returnType = signature->as <FunctionSignature> ()->getReturnType();
				node->setResultType(returnType);

				resultFromExpression = false;
			}

			else if(oneSided->type == OneSidedOperator::Type::Subscript)
			{
				assert(false && "Subscript not implemented");
			}

			// Only if there is an expression, validate it. In case such as
			// CallOperator there might be no expression due to empty brackets.
			if(oneSided->getExpression())
			{
				// Validate the expression of the one sided operator.
				if(!validateExpression(oneSided->getExpression()))
				{
					return false;
				}
			}

			// Use the type of the expression node.
			if(resultFromExpression)
			{
				node->setResultType(oneSided->getExpression()->getResultType().lock());
			}

			// TODO: Make sure that the expression node supports the given operator.
			break;
		}
	}

	return true;
}

bool Validator::validateScope(std::shared_ptr <ScopeDefinition> node)
{
	// If the scope is a function, do special validation.
	if(node->type == ScopeDefinition::Type::FunctionDefinition)
	{
		auto function = node->as <FunctionDefinition> ();

		// First validate the function.
		if(!function->validate(*this))
		{
			return false;
		}

		// When the function validation is done, the return type should be set.
		if(!function->getSignature()->getReturnType())
		{
			events.emit(ErrorMessage("Return value couldn't be deduced", node->name));
			return false;
		}
	}

	// For anything else just validate the root.
	else
	{
		if(!validateNode(node->getRoot()))
		{
			return false;
		}

		node->complete();
	}

	return true;
}

Reference Validator::resolveDefinition(std::shared_ptr <Expression> node)
{
	switch(node->type)
	{
		// Resolving a definition from an operator should only happen when the operator is an access.
		case Expression::Type::Operator:
		{
			assert(node->as <Operator> ()->type == Operator::Type::TwoSided);
			assert(node->as <TwoSidedOperator> ()->type == TwoSidedOperator::Type::Access);

			return Reference(resolveAccess(node->as <TwoSidedOperator> ()));
		}

		// If the node is a value, try to find a definition from the current scope.
		case Expression::Type::Value:
		{
			return getDefinition(node->token, getCurrentScope(node));
		}

		default:
		{
			assert(false);
		}
	}

	return Reference();
}

Reference Validator::resolveAccess(std::shared_ptr <TwoSidedOperator> node)
{
	assert(node->type == TwoSidedOperator::Type::Access);
	std::shared_ptr <TypeDefinition> context;

	switch(node->getLeft()->type)
	{
		// If the left node is an operator the context becomes the result type of the operator.
		case Expression::Type::Operator:
		{
			// TODO: Allow things such as "(call()).something".
			assert(node->getLeft()->as <Operator> ()->type == Operator::Type::TwoSided);

			auto leftDefinition = resolveAccess(node->getLeft()->as <TwoSidedOperator> ());
			context = getDefinitionType(leftDefinition);

			if(!context)
			{
				return Reference();
			}

			break;
		}

		// If the left node is an identifier value, it's the very first access location.
		case Expression::Type::Value:
		{
			if(node->getLeft()->token.getType() != Token::Type::Identifier)
			{
				events.emit(ErrorMessage("Expected an identifier before dot", node->getLeft()->token));
				return Reference();
			}

			// Current scope can be assumed as this value is the first access location.
			auto leftDefinition = getDefinition(node->getLeft()->token, getCurrentScope(node->getLeft()));
			context = getDefinitionType(leftDefinition);

			if(!context)
			{
				return Reference();
			}

			break;
		}

		default:
		{
			assert(false);
		}
	}

	// The right side node of access has to be an identifier.
	if(node->getRight()->type != Expression::Type::Value ||
		node->getRight()->token.getType() != Token::Type::Identifier)
	{
		if(node->getRight()->token.getType() != Token::Type::Identifier)
		{
			events.emit(ErrorMessage("Expected an identifier after dot", node->getLeft()->token));
			return Reference();
		}
	}

	assert(context);
	node->getLeft()->setResultType(context);

	// Now that the type pointed at by the left side is known, find the definition
	// pointed at by the right side using the left side as the context.
	auto rightDefinition = getDefinition(node->getRight()->token, context);
	node->getRight()->setResultType(getDefinitionType(rightDefinition));

	if(node->getRight()->getResultType().expired())
	{
		return Reference();
	}

	node->setResultType(node->getRight()->getResultType().lock());

	return rightDefinition;
}

std::shared_ptr <ScopeDefinition> Validator::getCurrentScope(std::shared_ptr <Node> root)
{
	assert(root);
	auto current = root;

	// While we're not in a scope definition and there is a parent, switch to the parent.
	while(current->type != Node::Type::ScopeDefinition && !current->getParent().expired())
	{
		current = current->getParent().lock();
	}

	// If the current node is a scope definition, return it.
	if(current->type == Node::Type::ScopeDefinition)
	{
		return current->as <ScopeDefinition> ();
	}

	return nullptr;
}

std::shared_ptr <ScopeDefinition> Validator::getCurrentNamedScope(std::shared_ptr <Node> root)
{
	auto scope = getCurrentScope(root);

	while(scope->name == "")
	{
		assert(!scope->getParent().expired());
		scope = getCurrentScope(scope->getParent().lock());
	}

	return scope;
}

Reference Validator::getDefinition(Token name, std::shared_ptr <ScopeDefinition> context)
{
	assert(name.getType() == Token::Type::Identifier);
	assert(context);

	events.emit(DebugMessage("Find definition " + name.getString() + " from " + context->name.getString(), name));

	// Try to find the definition in the current scope or one of its parents.
	auto definition = context->findDefinition(name.getStringView());
	if(!definition)
	{
		events.emit(ErrorMessage("Unknown identifier " + name.getString(), name));
		return Reference();
	}

	events.emit(DebugMessage("Found definition " + name.getString() + + " " + definition.getTypeString(), name));
	return Reference(definition);
}

std::shared_ptr <TypeDefinition> Validator::getDefinitionType(Reference reference)
{
	if(!reference)
	{
		return nullptr;
	}

	switch(reference.getType())
	{
		case Reference::Type::Parameter:
		case Reference::Type::Variable:
		{
			bool recursiveUsage = false;

			if(!reference.getAssociatedType())
			{
				inValidation.push_back(reference.getReferred());

				// Count how many times the definition has started validation.
				size_t occurences = std::count(inValidation.begin(), inValidation.end(), reference.getReferred());

				// If there's more than one active validation, recursive initializations are being done.
				if(occurences > 1)
				{
					recursiveUsage = true;
				}

				else
				{
					// Try to validate the referenced initialization.
					if(!validateExpressionRoot(reference.getReferred()->as <ExpressionRoot> ()))
					{
						return nullptr;
					}

					// The definition should now be validated.
					inValidation.pop_back();
				}
			}

			// If recursive initialization is being done, show an error.
			if(recursiveUsage || !reference.getAssociatedType())
			{
				events.emit(ErrorMessage("Unable to recursively use " + reference.getReferred()->token.getString(), reference.getReferred()->token));
				return nullptr;
			}

			// Return the type of the definition.
			return reference.getAssociatedType();
		}

		case Reference::Type::FunctionDefinition:
		{
			auto function = reference.getReferred()->as <FunctionDefinition> ();

			// If no signature exists or no return type is initialize, maybe validate the function.
			if(!function->getSignature() || !function->getSignature()->getReturnType())
			{
				auto it = std::find(inValidation.begin(), inValidation.end(), function);
				bool isBeingValidated = it != inValidation.end();

				// Only if the function isn't already in validation, try to validate it.
				if(!isBeingValidated)
				{
					inValidation.emplace_back(function);
					if(!function->validate(*this))
					{
						return nullptr;
					}

					inValidation.pop_back();
				}
			}

			return function->getSignature();
		}

		case Reference::Type::TypeDefinition:
		{
			return reference.getAssociatedType();
		}

		case Reference::Type::None: {}
	}

	return nullptr;
}

}
