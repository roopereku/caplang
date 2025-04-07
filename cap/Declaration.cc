#include <cap/Declaration.hh>
#include <cap/Function.hh>

#include <cassert>

namespace cap
{

Declaration::Declaration(Type type)
	: Node(Node::Type::Declaration), type(type)
{
}

const std::wstring& Declaration::getName() const
{
	return name;
}

Declaration::Type Declaration::getType()
{
	return type;
}

const TypeContext& Declaration::getReferredType() const
{
	return referredType;
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

Declaration::Root::Root(Type type)
	: Expression::Root(Expression::Type::DeclarationRoot), type(type)
{
}

Declaration::Root::Type Declaration::Root::getType() const
{
	return type;
}

const char* Declaration::Root::getTypeString(Type type)
{
	assert(type != Type::None);

	switch(type)
	{
		case Type::Generic: return "Generic declaration";
		case Type::Parameter: return "Parameter declaration";
		case Type::Local: return "Local declaration";

		default: {}
	}

	return "(declroot) ???";
}

const char* Declaration::Root::getTypeString() const
{
	return getTypeString(type);
}

}
