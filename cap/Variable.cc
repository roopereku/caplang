#include <cap/Variable.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Validator.hh>
#include <cap/Value.hh>

#include <cassert>

namespace cap
{

Variable::Variable(Type type, std::weak_ptr <BinaryOperator> initialization) :
	Declaration(Declaration::Type::Variable),
	type(type), initialization(initialization)
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
	if(!referredType.has_value())
	{
		referredType.emplace(TypeContext());

		assert(!initialization.expired());
		auto init = initialization.lock();

		// Validate the name and the initialization.
		if(!validator.traverseExpression(init))
		{
			return false;
		}

		// TODO: Should a declaration be referred to in some case?
		referredType.emplace(init->getResultType());
	}

	return true;
}

std::shared_ptr <Expression> Variable::getInitialization()
{
	assert(!initialization.expired());
	return initialization.lock()->getRight();
}

const char* Variable::getTypeString(Type type)
{
	switch(type)
	{
		case Type::Generic: return "Generic";
		case Type::Parameter: return "Parameter";
		case Type::Local: return "Local variable";

		default: {}
	}

	return "(declroot) ???";
}

const char* Variable::getTypeString() const
{
	return getTypeString(type);
}

Variable::Root::Root(Variable::Type type)
	: Expression::Root(Expression::Type::VariableRoot), type(type)
{
}

Variable::Type Variable::Root::getType() const
{
	return type;
}

const char* Variable::Root::getTypeString() const
{
	return "Variable root";
}

}
