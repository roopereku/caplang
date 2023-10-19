#include <cap/node/DeclarationReference.hh>
#include <cap/node/VariableDefinition.hh>
#include <cap/node/TypeDeclaration.hh>
#include <cap/node/FunctionDeclaration.hh>
#include <cap/Type.hh>

namespace cap
{

Type& DeclarationReference::getResultType()
{
	if(declaration->isVariableDefinition())
	{
		return declaration->as <VariableDefinition> ()->getVariable().getType();
	}

	else if(declaration->isType())
	{
		return *declaration->as <TypeDeclaration> ()->type;
	}

	else if(declaration->isFunction())
	{
		return declaration->as <FunctionDeclaration> ()->function->getReturnType();
	}

	return Expression::getResultType();
}

}
