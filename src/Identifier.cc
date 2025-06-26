#include <cap/Identifier.hh>
#include <cap/Declaration.hh>

#include <cassert>

namespace cap
{

Identifier::Identifier(std::wstring&& value)
	: Value(Type::Identifier), value(std::move(value))
{
}

const std::wstring& Identifier::getValue() const
{
	return value;
}

std::wstring Identifier::getString() const
{
	return getValue();
}

std::shared_ptr <Declaration> Identifier::getReferred()
{
	if(!referred.expired())
	{
		return referred.lock();
	}

	return nullptr;
}

void Identifier::setReferred(std::shared_ptr <Declaration> node)
{
	referred = node;
}

void Identifier::updateResultType()
{
    assert(!referred.expired());
    auto referredDecl = referred.lock();

    assert(referredDecl->getReferredType());
    setResultType(*referredDecl->getReferredType());
}

const char* Identifier::getTypeString() const
{
	return "Identifier";
}

}
