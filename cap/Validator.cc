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
	if(node->type == ExpressionRoot::Type::ImportStatement)
	{
		events.emit(ErrorMessage(std::string("Validation unimplemented for imports"), node->token));
		return false;
	}

	// If there's no root, do no validation on it.
	if(!node->getRoot())
	{
		// Return with no following expression indicates an implicit void return type.
		if(node->type == ExpressionRoot::Type::ReturnStatement)
		{
			node->setReference(Reference(PrimitiveType::getVoid()));
			return validateReturn(node->as <ReturnStatement> (), true);
		}

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

	// Handle return statements that precede a root. In this case void as a type is disallowed.
	if(node->type == ExpressionRoot::Type::ReturnStatement)
	{
		return validateReturn(node->as <ReturnStatement> (), false);
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

		// Explicit return type can't be used for constructors.
		if(scope->as <FunctionDefinition> ()->isConstructor())
		{
			events.emit(ErrorMessage("Explicit return type cannot be defined for a constructor", node->token));
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
		if(!signature->setReturnType(node->getResultType().lock(), *this))
		{
			return false;
		}
	}

	// Make sure that there aren't multiple initilizations of the same name in the same scope.
	else if(node->type == ExpressionRoot::Type::VariableDefinition ||
			node->type == ExpressionRoot::Type::ParameterDefinition ||
			node->type == ExpressionRoot::Type::AliasDefinition)
	{
		// If there's a definition of the same name as node, other than node
		// that's in the same scope as node, there's a colliding initialization.
		if(!checkNameCollision(node->token, node))
		{
			return false;
		}

		// TODO: Improve the check.
		// If the result type isn't set, there probably is usage of an uninitialized definition.
		if(node->getResultType().expired())
		{
			events.emit(ErrorMessage("Unable to initialize with a later defined definition", node->getRoot()->token));
			return false;
		}

		// Forbid assignment of void to variables and parameters.
		if(!node->getResultType().expired() &&
			node->getResultType().lock() == PrimitiveType::getVoid())
		{
			if(node->type == ExpressionRoot::Type::VariableDefinition ||
				node->type == ExpressionRoot::Type::ParameterDefinition)
			{
				events.emit(ErrorMessage("Unable to initialize with void", node->token));
				return false;
			}
		}
	}

	return true;
}

bool Validator::validateReturn(std::shared_ptr <ReturnStatement> node, bool allowVoid)
{
	// The current named scope has to be a function for return statements.
	auto scope = getCurrentNamedScope(node);
	if(scope->type != ScopeDefinition::Type::FunctionDefinition)
	{
		events.emit(ErrorMessage("Return statement outside a function", node->token));
		return false;
	}

	// Return statements within constuctors cannot return with an expression.
	if(!allowVoid && scope->as <FunctionDefinition> ()->isConstructor())
	{
		events.emit(ErrorMessage("Constructor shouldn't return a value", node->token));
		return false;
	}

	// If the expression of the return doesn't return any type, don't do checks.
	// Such could happen when an incomplete function is called.
	if(!node->getResultType().expired())
	{
		auto signature = scope->as <FunctionDefinition> ()->getSignature();
		auto resultType = node->getResultType().lock();

		// Disallow returning a void type unless said otherwise.
		if(!allowVoid && resultType == PrimitiveType::getVoid())
		{
			events.emit(ErrorMessage("Void used with return", node->token));
			return false;
		}

		// Set or update the return type.
		if(!signature->setReturnType(resultType, *this))
		{
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

	return node->validate(*this);
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
		// Make sure that the name of the scope doesn't collide with anything.
		if(node->name.getType() != Token::Type::Invalid && !checkNameCollision(node->name, node))
		{
			return false;
		}

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

std::shared_ptr <ScopeDefinition> Validator::getParentScope(std::shared_ptr <Node> root)
{
	auto scope = getCurrentScope(root);

	// Until getCurrentScope returns something than the given root, get the scope of parent.
	while(scope == root)
	{
		scope = getCurrentScope(scope->getParent().lock());
	}

	return scope;
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
	return definition;
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
		case Reference::Type::Alias:
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
			if(!function->isValidationComplete())
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

bool Validator::checkNameCollision(Token name, std::shared_ptr <Node> context)
{
	// Get the parent scope of context and check if another entity of the same name
	// exists. The given context node is excluded so that it won't match.
	auto containingScope = getParentScope(context);
	auto definition = containingScope->findDefinition(name.getStringView(), context);

	// If there was a match and it has the same parent scope as the
	// context node, there are colliding initialization names.
	if(definition && containingScope == getParentScope(definition.getReferred()))
	{
		events.emit(ErrorMessage("Colliding names", definition.getReferredName()));
		return false;
	}

	return true;
}

}
