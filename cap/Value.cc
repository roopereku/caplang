#include <cap/Value.hh>

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

}
