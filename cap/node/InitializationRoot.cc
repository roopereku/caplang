#include <cap/node/InitializationRoot.hh>
#include <cap/node/VariableDefinition.hh>
#include <cap/node/ParameterDefinition.hh>

#include <cassert>

namespace cap
{

InitializationRoot::InitializationRoot(Token& token)
	: ExpressionRoot(ExpressionRoot::Type::InitializationRoot, token), type(getType(token))
{
}

InitializationRoot::InitializationRoot(Token& token, Type type)
	: ExpressionRoot(ExpressionRoot::Type::InitializationRoot, token), type(type)
{
}

InitializationRoot::Type InitializationRoot::getType(Token& token)
{
	assert(token == Token::Type::Identifier);

	if(token == "var")
	{
		return Type::Variable;
	}

	else if(token == "alias")
	{
		return Type::Alias;
	}

	assert(false);
}

std::shared_ptr <ExpressionRoot> InitializationRoot::createDefinition(std::shared_ptr <Value> name,
																		std::shared_ptr <Expression> initialization)
{
	std::shared_ptr <ExpressionRoot> definition;

	switch(type)
	{
		case Type::Variable:
		{
			definition = std::make_shared <VariableDefinition> (name, initialization);
			break;
		}

		case Type::Parameter:
		{
			definition = std::make_shared <ParameterDefinition> (name, initialization);
			break;
		}

		case Type::Alias:
		{
			assert(false && "Aliases not implemented");
			break;
		}
	}

	definition->adopt(name);
	definition->adopt(initialization);

	return definition;
}

}
