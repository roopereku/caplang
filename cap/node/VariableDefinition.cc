#include <cap/node/VariableDefinition.hh>
#include <cap/node/TwoSidedOperator.hh>

#include <cassert>

namespace cap
{

VariableDefinition::VariableDefinition(Token& token)
	: ExpressionRoot(ExpressionRoot::Type::VariableDefinition, token)
{
}

std::shared_ptr <Node> VariableDefinition::isDefinition(Token name)
{
	assert(getRoot()->type == Expression::Type::Operator);
	assert(getRoot()->as <Operator> ()->type == Operator::Type::TwoSided);

	return findVariableNode(getRoot()->as <Expression> (), name);
}

std::shared_ptr <Expression> VariableDefinition::findVariableNode(std::shared_ptr <Expression> node, Token name)
{
	auto twoSided = getRoot()->as <TwoSidedOperator> ();

	switch(twoSided->type)
	{
		case TwoSidedOperator::Type::Assignment:
		{
			// The left node of an assignment should be a value.
			assert(twoSided->getLeft()->type == Expression::Type::Value);

			// If the name of the variable name matches the given name, return left.
			if(twoSided->getLeft()->token == name)
			{
				return twoSided->getLeft();
			}

			break;
		}

		case TwoSidedOperator::Type::Comma:
		{
			// If the left node of a comma contains the variable, return it.
			auto result = findVariableNode(twoSided->getLeft(), name);
			if(result)
			{
				return result;
			}

			// If the right node of a comma contains the variable, return it.
			result = findVariableNode(twoSided->getRight(), name);
			if(result)
			{
				return result;
			}

			break;
		}

		default:
		{
			assert(false);
		}
	}

	// Variable node wasn't found.
	return nullptr;
}

}
