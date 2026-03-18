#include <cap/TypeDefinition.hh>

namespace cap
{

TypeDefinition::TypeDefinition(Type type) :
    m_type(type)
{
}

TypeDefinition::Type TypeDefinition::getType()
{
    return m_type;
}

} // namespace cap
