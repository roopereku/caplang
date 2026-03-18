#include <cap/TypeContext.hh>
#include <cap/TypeDefinition.hh>

namespace cap
{

TypeContext::TypeContext(TypeDefinition& referenced) :
    m_referenced(referenced)
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
    return &m_referenced == &other.m_referenced;
}

std::wstring TypeContext::toString() const
{
    std::wstring modifiers;

    // TODO: Add the rest of the modfiers.
    return modifiers + m_referenced.toString(true);
}

} // namespace cap
