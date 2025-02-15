#include <cap/CallableType.hh>

#include <cassert>

namespace cap
{

CallableType::CallableType() :
	TypeDefinition(Type::Callable)
{
}

std::shared_ptr <Expression::Root> CallableType::getParameters()
{
	return parameters;
}

std::shared_ptr <Expression::Root> CallableType::getReturnType()
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

const char* CallableType::getTypeString()
{
	return "Callable Type";
}

}
