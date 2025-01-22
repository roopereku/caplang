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

void Declaration::Root::setParameterDeclaration()
{
	parameterDeclaration = true;
}

bool Declaration::Root::isParameterDeclaration() const
{
	return parameterDeclaration;
}

std::shared_ptr <Scope> Declaration::Root::findTargetScope()
{
	// TODO: If a parent of this declaration node is a scoped expression
	// such as "if", "when" or "while", return something more specific.

	// Return the function body for parameter declarations.
	if(parameterDeclaration)
	{
		auto parent = getParentFunction();
		assert(parent);

		return parent->getBody();
	}

	return getParentScope();
}

const char* Declaration::Root::getTypeString()
{
	return parameterDeclaration ? "Parameter Root" : "Declaration Root";
}

}
