#include <cap/ParameterAccessor.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Declaration.hh>

#include <cassert>

namespace cap
{

ParameterAccessor::ParameterAccessor(std::shared_ptr <Expression::Root> root)
{
	assert(root);

	if(root->getFirst())
	{
		hasDeclarations = root->getFirst()->getType() == Expression::Type::DeclarationRoot;

		if(hasDeclarations)
		{
			current = std::static_pointer_cast <Declaration::Root> (root->getFirst())->getFirst();
		}

		else
		{
			current = root->getFirst();
		}
	}
}

std::shared_ptr <Expression> ParameterAccessor::getNext()
{
	auto result = current;
	bool switched = false;

	if(result)
	{
		if(current->getType() == Expression::Type::BinaryOperator)
		{
			auto op = std::static_pointer_cast <BinaryOperator> (current);

			// If the current node is a comma, move on to the next one.
			if(op->getType() == BinaryOperator::Type::Comma)
			{
				result = op->getLeft();
				current = op->getRight();
				switched = true;
			}
		}

		if(!switched)
		{
			// If there was no comma, there is also no next parameter.
			current = nullptr;
		}

		if(hasDeclarations)
		{
			assert(result->getType() == Expression::Type::BinaryOperator);
			auto op = std::static_pointer_cast <BinaryOperator> (result);

			assert(op->getType() == BinaryOperator::Type::Assign);
			result = op->getLeft();
		}
	}

	return result;
}

}
