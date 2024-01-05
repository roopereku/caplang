#include <cap/Validator.hh>

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
	assert(node);

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
			// TODO: Handle values.
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

			break;
		}

		case Operator::Type::OneSided:
		{
			auto oneSided = node->as <OneSidedOperator> ();

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

				// TODO: Get the type from the leftmost node of the initialization.

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

std::shared_ptr <Expression> Validator::getLeftmostExpression(std::shared_ptr <Expression> node)
{
	assert(node->type == Expression::Type::Operator);

	switch(node->as <Operator> ()->type)
	{
		case Operator::Type::TwoSided:
			return getLeftmostExpression(node->as <TwoSidedOperator> ()->getLeft());

		case Operator::Type::OneSided:
			return getLeftmostExpression(node->as <OneSidedOperator> ()->getExpression());
	}
}

}
