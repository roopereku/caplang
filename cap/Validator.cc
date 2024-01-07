#include <cap/Validator.hh>

#include <cap/node/Value.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>

#include <cap/event/ErrorMessage.hh>
#include <cap/event/DebugMessage.hh>

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
				auto definition = getDefinition(node->as <Value> (), getCurrentScope(node));
				if(!definition)
				{
					return false;
				}

				switch(definition->type)
				{
					case Node::Type::ScopeDefinition:
					{
						events.emit(ErrorMessage("Unimplemented: Scope as a definition result", node->token));
						return false;
					}

					case Node::Type::Expression:
					{
						// If the definition result is an expression, it should be a variable name node.
						assert(definition->as <Expression> ()->type == Expression::Type::Value);

						// If no type is set for the variable which is found, it is an incomplete variable.
						if(definition->as <Value> ()->getResultType().expired())
						{
							events.emit(ErrorMessage("Unable to use an incomplete variable", node->token));
							return false;
						}

						// Use the type of a variable
						node->setResultType(definition->as <Value> ()->getResultType().lock());

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
	switch(node->type)
	{
		case ExpressionRoot::Type::Expression:
		{
			if(!validateExpression(node->getRoot()))
			{
				return false;
			}

			break;
		}

		case ExpressionRoot::Type::VariableDefinition:
		{
			if(!validateVariableInit(node->getRoot()))
			{
				return false;
			}

			break;
		}
	}

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
			// Validate the expression of the one sided operator.
			if(!validateExpression(node->as <OneSidedOperator> ()->getExpression()))
			{
				return false;
			}

			// TODO: Make sure that the expression node supports the given operator.
			// TODO: Use the type of expression in this node.

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

	return true;
}

bool Validator::validateVariableInit(std::shared_ptr <Expression> node)
{
	assert(node->type == Expression::Type::Operator);
	bool noAssignment = false;

	// The given operator node should be two sided.
	if(node->as <Operator> ()->type == Operator::Type::TwoSided)
	{
		auto twoSided = node->as <TwoSidedOperator> ();

		switch(twoSided->type)
		{
			// Comma is allowed as it splits multiple variable definitions.
			case TwoSidedOperator::Type::Comma:
			{
				// Both sides of the comma have to initialize a variable.
				if(!validateVariableInit(twoSided->getLeft()) ||
					!validateVariableInit(twoSided->getRight()))
				{
					return false;
				}

				break;
			}

			// Assignment indicates variable initialization.
			case TwoSidedOperator::Type::Assignment:
			{
				// The variable name has to be a value.
				if(twoSided->getLeft()->type != Expression::Type::Value)
				{
					events.emit(ErrorMessage("Cannot apply operators to a variable name", twoSided->getLeft()->token));
					return false;
				}

				// The variable name has to be an identifier.
				else if(twoSided->getLeft()->token.getType() != Token::Type::Identifier)
				{
					events.emit(ErrorMessage("Expected an identifier as a variable name", twoSided->getLeft()->token));
					return false;
				}

				// Validate the initialization.
				if(!validateExpression(twoSided->getRight()))
				{
					return false;
				}

				// Get the result type of the initialization.
				auto resultType = twoSided->getRight()->getResultType();
				//assert(!resultType.expired());

				if(!resultType.expired())
				{
					// Save the result type of the initialization to the variable name node.
					twoSided->getLeft()->setResultType(resultType.lock());
				}

				break;
			}

			// Any other operator type is forbidden as the
			// first node in variable initialization.
			default:
			{
				noAssignment = true;
			}
		}
	}

	// The given node isn't a two sided operator.
	else
	{
		noAssignment = true;
	}

	// If there current node doesn't initialize a variable, throw an error.
	if(noAssignment)
	{
		events.emit(ErrorMessage("Expected '=' after variable name", node->token));
		return false;
	}

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

	if(!node->getRight())
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
		assert(definition->as <Expression> ()->type == Expression::Type::Value);
		assert(!definition->as <Expression> ()->getResultType().expired());

		return definition->as <Expression> ()->getResultType().lock();
	}

	else if(definition->type == Node::Type::ScopeDefinition)
	{
		assert(definition->as <ScopeDefinition> ()->type == ScopeDefinition::Type::TypeDefinition);
	}

	return definition->as <TypeDefinition> ();
}

}
