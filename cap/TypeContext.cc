#include <cap/TypeContext.hh>
#include <cap/TypeDefinition.hh>

namespace cap
{

TypeContext::TypeContext(std::shared_ptr <TypeDefinition> referenced)
	: isImmutable(false), isParseTime(false), isTypeName(false), referenced(referenced)
{
}

TypeContext::TypeContext() : TypeContext(nullptr)
{
}

std::shared_ptr <TypeDefinition> TypeContext::getReferenced() const
{
	return referenced.expired() ? nullptr : referenced.lock();
}

std::wstring TypeContext::toString() const
{
	// TODO: Add modifiers as well.
	auto ref = getReferenced();
	return ref ? ref->getLocation() : L"";
}

}
