#include <cap/Variable.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Validator.hh>
#include <cap/Value.hh>

#include <cassert>

namespace cap
{

Variable::Variable(std::weak_ptr <BinaryOperator> initialization)
	: Declaration(Type::Variable), initialization(initialization)
{
	assert(!initialization.expired());

	auto node = initialization.lock();
	assert(node->getLeft()->getType() == Expression::Type::Value);

	auto nameValue = std::static_pointer_cast <Value> (node->getLeft());
	name = nameValue->getValue();

	setToken(nameValue->getToken());

	// TODO: If the initialization is prefixed with "type", set the type as a type alias.
}

bool Variable::validate(Validator& validator)
{
	if(!referredType.getReferenced())
	{
		// TODO: Initialize referredType earlier somehow to prevent recursion?

		assert(!initialization.expired());
		auto op = initialization.lock();

		assert(op->getType() == BinaryOperator::Type::Assign);
		if(!validator.traverseExpression(op->getRight()))
		{
			return false;
		}

		// TODO: What exactly is being referred to here?
		referredType = op->getRight()->getResultType();
	}

	return true;
}

}
