#include <cap/CallableType.hh>
#include <cap/ArgumentAccessor.hh>
#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

CallableType::CallableType() :
	TypeDefinition(Type::Callable)
{
	name = L"callable";
}

std::shared_ptr <Variable::Root> CallableType::getParameterRoot() const
{
	return parameters;
}

std::shared_ptr <Expression::Root> CallableType::getReturnTypeRoot() const
{
	return returnType;
}

void CallableType::initializeParameters()
{
	assert(!parameters);
	parameters = std::make_shared <Variable::Root> (Variable::Type::Parameter);
}

void CallableType::initializeReturnType()
{
	assert(!returnType);
	returnType = std::make_shared <Expression::Root> ();
}

std::pair <bool, size_t> CallableType::matchParameters(ArgumentAccessor&& arguments) const
{
	ArgumentAccessor self(parameters);
	size_t unidentical = 0;

	while(auto selfCurrent = self.getNext())
	{
		// If other doesn't have anything to match, the parameter counts differ.
		auto otherCurrent = arguments.getNext();
		if(!otherCurrent)
		{
			return { false, 0 };
		}

		bool identical = selfCurrent->getResultType().isIdentical(otherCurrent->getResultType());
		bool compatible = selfCurrent->getResultType().isCompatible(otherCurrent->getResultType());

		if(!compatible)
		{
			return { false, 0 };
		}

		unidentical += !identical;
	}

	// If other still has something, the parameter counts differ.
	return { arguments.getNext() == nullptr, unidentical };
}

bool CallableType::validate(Validator& validator)
{
	if(!referredType.has_value())
	{
		referredType = TypeContext(std::static_pointer_cast <CallableType> (shared_from_this()));
		referredType.value().isTypeName = true;

		return validator.traverseTypeDefinition(std::static_pointer_cast <CallableType> (shared_from_this()));
	}

	return true;
}

const char* CallableType::getTypeString() const
{
	return "Callable Type";
}

}
