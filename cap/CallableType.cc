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

std::shared_ptr <Expression::Root> CallableType::getParameterRoot() const
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
	parameters = std::make_shared <Expression::Root> ();
}

void CallableType::initializeReturnType()
{
	assert(!returnType);
	returnType = std::make_shared <Expression::Root> ();
}

std::pair <bool, size_t> CallableType::matchParameters(std::shared_ptr <Expression::Root> root) const
{
	ArgumentAccessor self(parameters);
	ArgumentAccessor other(root);

	size_t unidentical = 0;

	while(auto selfCurrent = self.getNext())
	{
		// If other doesn't have anything to match, the parameter counts differ.
		auto otherCurrent = other.getNext();
		if(!otherCurrent)
		{
			return { false, 0 };
		}

		// Are the current parameters compatible with each other?
		auto [compatible, identical] = isCompatible(selfCurrent->getResultType(), otherCurrent->getResultType());
		if(!compatible)
		{
			return { false, 0 };
		}

		unidentical += !identical;
	}

	// If other still has something, the parameter counts differ.
	return { other.getNext() == nullptr, unidentical };
}

std::pair <bool, bool> CallableType::isCompatible(const TypeContext& selfCtx, const TypeContext& otherCtx) const
{
	auto ref1 = selfCtx.getReferenced();
	auto ref2 = otherCtx.getReferenced();

	assert(ref1);
	assert(ref2);

	// TODO: Instead of just checking for equality, return true if the types are compatible.
	// TODO: Check for other modifiers in the type context.
	bool identical = ref1 == ref2;
	return { identical, identical };
}

bool CallableType::validate(Validator& validator)
{
	if(!referredType.getReferenced())
	{
		referredType = TypeContext(std::static_pointer_cast <CallableType> (shared_from_this()));
		referredType.isTypeName = true;

		return validator.traverseTypeDefinition(std::static_pointer_cast <CallableType> (shared_from_this()));
	}

	return true;
}

const char* CallableType::getTypeString() const
{
	return "Callable Type";
}

}
