#include <cap/String.hh>

namespace cap
{

// TODO: The value initialization might not be enough for multiline strings
// depending on how they are implemented.
String::String(std::wstring&& value) :
	Value(Type::String),
	value(value.begin() + 1, value.end() - 1)
{
}

const std::wstring& String::getValue() const
{
	return value;
}

const char* String::getTypeString() const
{
	return "String";
}

}
