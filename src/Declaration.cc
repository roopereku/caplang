#include <cap/Declaration.hh>
#include <cap/Function.hh>
#include <cap/ParserContext.hh>
#include <cap/Validator.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

Declaration::Declaration(Type type)
	: Node(Node::Type::Declaration), m_type(type)
{
}

Declaration::Declaration(Type type, DeclarationStorage& declStorage)
	: Node(Node::Type::Declaration, declStorage), m_type(type)
{
}

const std::wstring& Declaration::getName() const
{
	return m_name;
}

Declaration::Type Declaration::getType()
{
	return m_type;
}

const std::optional <TypeContext>& Declaration::getReferredType() const
{
	return m_referredType;
}

bool Declaration::validate(Validator& validator)
{
	return validateAttributes(validator);
}

std::wstring Declaration::getLocation(wchar_t delimiter) const
{
	auto parentDecl = getParentDeclaration();

	// If there's no parent, no need to delimit.
	if(!parentDecl)
	{
		return getName();
	}

	// Add the parent name if it's a declaration and known to have a name.
	return parentDecl->getLocation(delimiter) + delimiter + getName();
}

std::shared_ptr <Declaration> Declaration::getParentDeclaration() const
{
	auto current = getParent();
	while(!current.expired())
	{
		auto locked = current.lock();
		if(locked->getType() == Node::Type::Declaration)
		{
			return std::static_pointer_cast <Declaration> (locked);
		}

		current = locked->getParent();
	}

	return nullptr;
}

bool Declaration::isAttribute() const
{
	return m_attribute;
}

bool Declaration::handleBuiltinAttribute(Validator&, Builtin::AttributeType type, std::shared_ptr <Attribute>)
{
	switch(type)
	{
		case Builtin::AttributeType::Definition:
		{
			m_attribute = true;
			break;
		}
	}

	return true;
}

}
