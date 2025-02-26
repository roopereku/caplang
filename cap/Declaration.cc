#include <cap/Declaration.hh>
#include <cap/Function.hh>

#include <cassert>

namespace cap
{

Declaration::Declaration(Type type)
	: Node(Node::Type::Declaration), type(type)
{
}

const std::wstring& Declaration::getName()
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
