#include <cap/TypeContext.hh>
#include <cap/TypeDefinition.hh>

namespace cap
{

TypeContext::TypeContext(TypeDefinition& referenced)
	: referenced(referenced)
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
	return &referenced == &other.referenced;
}

std::wstring TypeContext::toString() const
{
	std::wstring modifiers;

	// TODO: Add the rest of the modfiers.
	return modifiers + referenced.toString(true);
}

}
