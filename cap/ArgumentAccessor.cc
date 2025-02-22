#include <cap/ArgumentAccessor.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Declaration.hh>

#include <cassert>

namespace cap
{

ArgumentAccessor::ArgumentAccessor(std::shared_ptr <Expression::Root> root)
{
	assert(root);

	if(root->getFirst())
	{
		// In the case of declararations, the stopping point is the declaration root.
		if(root->getFirst()->getType() == Expression::Type::DeclarationRoot)
		{
			origin = root->getFirst();
			current = std::static_pointer_cast <Declaration::Root> (origin)->getFirst();
		}

		// In the case of non-declarations, the stopping point the expression root.
		else
		{
			origin = root;
			current = root->getFirst();
		}

		// Find the value of the leftmost comma if any.
		// This will result in the first argument in case there are multiple.
		while(current->getType() == Expression::Type::BinaryOperator)
		{
			auto op = std::static_pointer_cast <BinaryOperator> (current);
			if(op->getType() == BinaryOperator::Type::Comma)
			{
				current = op->getLeft();
				continue;
			}

			break;
		}
	}
}

std::shared_ptr <Expression> ArgumentAccessor::getNext()
{
	// Are we at the stopping point?
	if(current == origin)
	{
		return nullptr;
	}

	auto result = current;
	assert(current);

	if(current->getType() == Expression::Type::BinaryOperator)
	{
		auto op = std::static_pointer_cast <BinaryOperator> (current);
		if(op->getType() == BinaryOperator::Type::Comma)
		{
			// Since the current will always be initially a non-comma node,
			// if the current node is a comma we can assume that the first
			// node which would be the only left side value is handled.
			result = op->getRight();
		}
	}

	// Move on to the next argument if commas are present. Else the next
	// getNext call should hit the origin.
	current = std::static_pointer_cast <Expression> (current->getParent().lock());
	return result;
}

}
