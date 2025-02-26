#include <cap/Value.hh>
#include <cap/Declaration.hh>

#include <cassert>

namespace cap
{

Value::Value(std::wstring&& value)
	: Expression(Type::Value), value(std::move(value))
{
}

bool Value::isComplete() const
{
	assert(false && "cap::Value::isComplete should never be called");
	return false;
}

const std::wstring& Value::getValue()
{
	return value;
}

std::shared_ptr <Declaration> Value::getReferred()
{
	if(!referred.expired())
	{
		return referred.lock();
	}

	return nullptr;
}

void Value::setReferred(std::shared_ptr <Declaration> node)
{
	referred = node;
	setResultType(node->getReferredType());
}

const char* Value::getTypeString() const
{
	return "Value";
}

}
