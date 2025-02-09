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

const char* Declaration::getTypeString()
{
	switch(type)
	{
		case Type::TypeDefinition: return "Type definition";
		case Type::Function: return "Function";
		case Type::Variable: return "Variable";
	}

	return "(decl) ???";
}

Declaration::Root::Root(Type type)
	: Expression::Root(Expression::Type::DeclarationRoot), type(type)
{
}

std::shared_ptr <Scope> Declaration::Root::findTargetScope()
{
	// TODO: If a parent of this declaration node is a scoped expression
	// such as "if", "when" or "while", return something more specific.

	// Return the function body for parameter declarations.
	if(type == Type::Parameter)
	{
		auto parent = getParentFunction();
		assert(parent);

		return parent->getBody();
	}

	return getParentScope();
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

const char* Declaration::Root::getTypeString()
{
	return getTypeString(type);
}

}
