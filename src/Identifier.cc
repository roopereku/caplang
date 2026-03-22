#include <cap/Declaration.hh>
#include <cap/Identifier.hh>

#include <cassert>

namespace cap
{

Identifier::Identifier(std::wstring&& value) :
    Value(Type::Identifier),
    m_value(std::move(value))
{
}

const std::wstring& Identifier::getValue()
{
    return m_value;
}

std::shared_ptr<Declaration> Identifier::getReferred()
{
    if (!m_referred.expired())
    {
        return m_referred.lock();
    }

    return nullptr;
}

void Identifier::setReferred(std::shared_ptr<Declaration> node)
{
    m_referred = node;
}

void Identifier::updateResultType()
{
    assert(!m_referred.expired());
    auto referredDecl = m_referred.lock();

    assert(referredDecl->getReferredType());
    setResultType(*referredDecl->getReferredType());
}

const char* Identifier::getTypeString() const
{
    return "Identifier";
}

} // namespace cap
