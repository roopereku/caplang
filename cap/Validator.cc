#include <cap/Validator.hh>

#include <cap/node/Value.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/VariableDefinition.hh>

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
			if(node->token == Token::Type::Identifier)
			{
				events.emit(DebugMessage("Find definition of value " + node->token.getString(), node->token));

				auto definition = getDefinition(node->as <Value> (), getCurrentScope(node));
				if(!definition)
				{
					return false;
				}

				switch(definition->type)
				{
					case Node::Type::ScopeDefinition:
					{
						assert(definition->as <ScopeDefinition> ()->type == ScopeDefinition::Type::TypeDefinition);

						// Use the type definition as the result type.
						node->setResultType(definition->as <TypeDefinition> ());
						break;
					}

					case Node::Type::Expression:
					{
						assert(definition->as <Expression> ()->type == Expression::Type::Root);
						assert(definition->as <ExpressionRoot> ()->type == ExpressionRoot::Type::VariableDefinition);

						// Try to get the type of the variable.
						auto definitionType = getDefinitionType(definition);
						if(!definitionType)
						{
							return false;
						}

						// Use the type of a variable.
						node->setResultType(definitionType);

						break;
					}

					default:
					{
						assert(false);
					}
				}
			}

			// The non-identifier token should be conversible to a primitive type.
			else
			{
				auto primitiveType = TypeDefinition::getPrimitive(node->token);

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
	// Validate the root expression node.
	if(!validateExpression(node->getRoot()))
	{
		return false;
	}

	// Set the result type of the expression root.
	node->setResultType(node->getRoot()->getResultType().lock());
	return true;
}

bool Validator::validateOperator(std::shared_ptr <Operator> node)
{
	switch(node->type)
	{
		case Operator::Type::TwoSided:
		{
			auto twoSided = node->as <TwoSidedOperator> ();

			// The access operator has a special meaning.
			if(twoSided->type == TwoSidedOperator::Type::Access)
			{
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

			// Validate the expression of the one sided operator.
			if(!validateExpression(oneSided->getExpression()))
			{
				return false;
			}

			// Use the type of the expression node.
			node->setResultType(oneSided->getExpression()->getResultType().lock());

			// TODO: Make sure that the expression node supports the given operator.

			break;
		}
	}

	return true;
}

bool Validator::validateScope(std::shared_ptr <ScopeDefinition> node)
{
	if(!validateNode(node->getRoot()))
	{
		return false;
	}

	node->complete();

	return true;
}

std::shared_ptr <Node> Validator::resolveAccess(std::shared_ptr <TwoSidedOperator> node)
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
				return nullptr;
			}

			break;
		}

		// If the left node is an identifier value, it's the very first access location.
		case Expression::Type::Value:
		{
			if(node->getLeft()->token.getType() != Token::Type::Identifier)
			{
				events.emit(ErrorMessage("Expected an identifier before dot", node->getLeft()->token));
				return nullptr;
			}

			// Current scope can be assumed as this value is the first access location.
			auto leftDefinition = getDefinition(node->getLeft()->as <Value> (), getCurrentScope(node->getLeft()));
			context = getDefinitionType(leftDefinition);

			if(!context)
			{
				return nullptr;
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
			return nullptr;
		}
	}

	assert(context);

	node->setResultType(context->as <TypeDefinition> ());
	node->getLeft()->setResultType(context);

	// Now that the type pointed at by the left side is known, find the definition
	// pointed at by the right side using the left side as the context.
	auto rightDefinition = getDefinition(node->getRight()->as <Value> (), context);
	node->getRight()->setResultType(getDefinitionType(rightDefinition));

	if(node->getRight()->getResultType().expired())
	{
		return nullptr;
	}

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

std::shared_ptr <Node> Validator::getDefinition(std::shared_ptr <Value> node,
												std::shared_ptr <ScopeDefinition> context)
{
	assert(node->token.getType() == Token::Type::Identifier);
	assert(context);

	events.emit(DebugMessage("Find definition " + node->token.getString() + " from " + context->name.getString(), context->token));

	// Try to find the definition in the current scope or one of its parents.
	auto definition = context->findDefinition(node->token);

	if(!definition)
	{
		events.emit(ErrorMessage("Unknown identifier " + node->token.getString(), node->token));
		return nullptr;
	}

	return definition;
}

std::shared_ptr <TypeDefinition> Validator::getDefinitionType(std::shared_ptr <Node> definition)
{
	if(!definition)
	{
		return nullptr;
	}

	// If the definition of left is a variable, use its type as the context.
	if(definition->type == Node::Type::Expression)
	{
		assert(definition->as <Expression> ()->type == Expression::Type::Root);
		assert(definition->as <ExpressionRoot> ()->type == ExpressionRoot::Type::VariableDefinition);

		bool recursiveUsage = false;

		if(definition->as <VariableDefinition> ()->getResultType().expired())
		{
			inValidation.push_back(definition);

			// Count how many times the definition has started validation.
			size_t occurences = std::count(inValidation.begin(), inValidation.end(), definition);

			// If there's more than one active validation, recursive initializations are being done.
			if(occurences > 1)
			{
				recursiveUsage = true;
			}

			else
			{
				// Try to validate the referenced variable.
				if(!validateExpressionRoot(definition->as <VariableDefinition> ()))
				{
					return nullptr;
				}

				// The definition should now be validated.
				inValidation.pop_back();
			}
		}

		// If recursive initialization is being done, show an error.
		if(recursiveUsage || definition->as <Expression> ()->getResultType().expired())
		{
			events.emit(ErrorMessage("Unable to recursively use " + definition->token.getString(), definition->token));
			return nullptr;
		}

		return definition->as <Expression> ()->getResultType().lock();
	}

	else if(definition->type == Node::Type::ScopeDefinition)
	{
		assert(definition->as <ScopeDefinition> ()->type == ScopeDefinition::Type::TypeDefinition);
	}

	return definition->as <TypeDefinition> ();
}

}
