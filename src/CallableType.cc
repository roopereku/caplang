#include <cap/CallableType.hh>
#include <cap/ArgumentAccessor.hh>
#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

CallableType::CallableType() :
	TypeDefinition(Type::Callable)
{
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

		assert(selfCurrent->getResultType());
		assert(otherCurrent->getResultType());

		bool identical = selfCurrent->getResultType()->isIdentical(*otherCurrent->getResultType());
		bool compatible = selfCurrent->getResultType()->isCompatible(*otherCurrent->getResultType());

		if(!compatible)
		{
			return { false, 0 };
		}

		unidentical += !identical;
	}

	// If other still has something, the parameter counts differ.
	return { arguments.getNext() == nullptr, unidentical };
}

std::wstring CallableType::toString(bool) const
{
	ArgumentAccessor params(parameters);
	bool firstAdded = false;
	std::wstring result(L"func(");

	while(auto current = params.getNext())
	{
		if(firstAdded)
		{
			result += L", ";
		}

		else
		{
			firstAdded = true;
		}

		assert(current->getResultType());
		result += current->getResultType()->toString();
	}

	return result + L") -> " + returnType->getResultType()->toString();
}

}
