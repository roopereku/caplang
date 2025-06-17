#include <cap/ArgumentAccessor.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Variable.hh>

#include <cassert>

namespace cap
{

ArgumentAccessor::ArgumentAccessor(std::shared_ptr <Expression::Root> root)
{
	assert(root);

	origin = root;
	current = root->getFirst();

	locateFirst();
}

ArgumentAccessor::ArgumentAccessor(std::shared_ptr <Variable::Root> root)
{
	assert(root);
	assert(root->getInitializer());

	origin = root->getInitializer();
	current = root->getInitializer()->getFirst();

	locateFirst();
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

void ArgumentAccessor::locateFirst()
{
	// Nothing to locate.
	if(current == nullptr)
	{
		origin = nullptr;
		return;
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
