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

bool TypeContext::isCompatible(const TypeContext& other) const
{
	// FIXME: Just checking type equality isn't sufficient.
	return isIdentical(other);
}

bool TypeContext::isIdentical(const TypeContext& other) const
{
	// TODO: Account for modifiers such as mutability.
	return getReferenced() == other.getReferenced();
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
